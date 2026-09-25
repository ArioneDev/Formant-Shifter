#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr float minShiftCents = -1200.0f;
constexpr float maxShiftCents = 1200.0f;
}

FormantShifterAudioProcessor::FormantShifterAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout FormantShifterAudioProcessor::createParameterLayout()
{
    using APVTS = juce::AudioProcessorValueTreeState;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("formantShift", "Formant Shift",
        juce::NormalisableRange<float> (minShiftCents, maxShiftCents, 1.0f), 0.0f, "ct"));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("envelope", "Envelope",
        juce::NormalisableRange<float> (0.0f, 16.0f, 0.01f), 10.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("dryWet", "Dry/Wet",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.01f), 100.0f, "%"));
    p.push_back (std::make_unique<juce::AudioParameterBool> ("limiter", "Limiter", true));
    return { p.begin(), p.end() };
}

void FormantShifterAudioProcessor::prepareToPlay (double sr, int)
{
    currentSampleRate = sr;
    inputFifo.setSize (2, fftSize, false, true, true);
    outputFifo.setSize (2, fftSize, true, true, true);
    fftData.setSize (2, fftSize * 2, true, true, true);
    envelope.assign (fftSize / 2 + 1, 0.0f);
    shiftedEnvelope.assign (fftSize / 2 + 1, 0.0f);
    frameMagnitudes.assign (fftSize / 2 + 1, 0.0f);
    spectralEnvelope.assign (fftSize / 2 + 1, 0.0f);
    fifoIndex = outputIndex = 0;
    shiftSmoothed.reset (sr, 0.02);
    envelopeSmoothed.reset (sr, 0.02);
    mixSmoothed.reset (sr, 0.02);
    shiftSmoothed.setCurrentAndTargetValue (readParameter ("formantShift"));
    envelopeSmoothed.setCurrentAndTargetValue (readParameter ("envelope"));
    mixSmoothed.setCurrentAndTargetValue (readParameter ("dryWet") / 100.0f);
}

void FormantShifterAudioProcessor::releaseResources() {}

bool FormantShifterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& in = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();
    return (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo())
        && in == out;
}

float FormantShifterAudioProcessor::readParameter (const char* id) const
{
    return parameters.getRawParameterValue (id)->load();
}

void FormantShifterAudioProcessor::processFrame (int channel)
{
    auto* data = fftData.getWritePointer (channel);
    auto* input = inputFifo.getReadPointer (channel);
    auto* output = outputFifo.getWritePointer (channel);
    std::fill (data, data + fftSize * 2, 0.0f);
    std::copy (input, input + fftSize, data);
    window.multiplyWithWindowingTable (data, fftSize);
    fft.performRealOnlyForwardTransform (data);

    const int bins = fftSize / 2 + 1;
    frameMagnitudes[0] = std::abs (data[0]) + 1.0e-9f;
    frameMagnitudes[(size_t) (bins - 1)] = std::abs (data[1]) + 1.0e-9f;
    for (int k = 1; k < bins - 1; ++k)
    {
        const float re = data[2 * k], im = data[2 * k + 1];
        const float magnitude = std::sqrt (re * re + im * im) + 1.0e-9f;
        frameMagnitudes[(size_t) k] = magnitude;
    }

    // This follows the gen~ codebox in fft-formant-shift-gen-m4l.maxpat:
    // detect one maximum in each width-sized region, interpolate those
    // maxima into an envelope, shift the envelope, then restore detail.
    const int envelopeBins = fftSize / 2;
    const int width = juce::jlimit (2, fftSize / 4,
                                    juce::roundToInt (envelopeSmoothed.getCurrentValue()));
    const int regionCount = (envelopeBins + width - 1) / width;
    std::vector<float> peakValues ((size_t) regionCount + 2, 0.0f);
    std::vector<int> peakIndices ((size_t) regionCount + 2, 0);
    std::vector<float> interpolated ((size_t) envelopeBins, 0.0f);

    peakValues[0] = frameMagnitudes[0];
    peakIndices[0] = 0;
    peakValues[(size_t) regionCount + 1] = frameMagnitudes[(size_t) envelopeBins - 1];
    peakIndices[(size_t) regionCount + 1] = envelopeBins - 1;

    for (int region = 0; region < regionCount; ++region)
    {
        const int first = region * width;
        const int last = juce::jmin (first + width, envelopeBins);
        float sum = 0.0f;
        for (int i = first; i < last; ++i)
            sum += frameMagnitudes[(size_t) i];
        const float average = sum / (float) juce::jmax (1, last - first);
        float localMaximum = 0.0f;
        int maximumIndex = first;
        for (int i = first; i < last; ++i)
        {
            const float current = frameMagnitudes[(size_t) i];
            if (current > average && current >= localMaximum)
            {
                localMaximum = current;
                maximumIndex = i;
            }
        }
        peakValues[(size_t) region + 1] = localMaximum;
        peakIndices[(size_t) region + 1] = maximumIndex;
    }

    for (int point = 1; point <= regionCount + 1; ++point)
    {
        const int start = peakIndices[(size_t) point - 1];
        const int end = peakIndices[(size_t) point];
        const float startValue = peakValues[(size_t) point - 1];
        const float endValue = peakValues[(size_t) point];
        if (end <= start)
        {
            if (start >= 0 && start < envelopeBins)
                interpolated[(size_t) start] = endValue;
            continue;
        }
        for (int i = start; i < end && i < envelopeBins; ++i)
        {
            const float t = (float) (i - start) / (float) (end - start);
            interpolated[(size_t) i] = startValue + t * (endValue - startValue);
        }
    }
    interpolated[(size_t) envelopeBins - 1] = peakValues[(size_t) regionCount + 1];

    const float ratio = std::pow (2.0f, juce::jlimit (-48.0f, 48.0f,
                                                       shiftSmoothed.getCurrentValue() / 100.0f) / 12.0f);
    for (int k = 0; k < bins; ++k)
    {
        const float source = juce::jlimit (0.0f, (float) (envelopeBins - 1), k / ratio);
        const int lo = (int) source;
        const int hi = juce::jmin (lo + 1, envelopeBins - 1);
        const float frac = source - (float) lo;
        const float shifted = interpolated[(size_t) lo] * (1.0f - frac)
                            + interpolated[(size_t) hi] * frac;
        const float originalEnvelope = juce::jmax (interpolated[(size_t) juce::jmin (k, envelopeBins - 1)],
                                                   1.0e-9f);
        const float gain = juce::jlimit (0.05f, 20.0f,
                                         shifted / originalEnvelope);
        if (k == 0)
        {
            data[0] *= gain;
        }
        else if (k == bins - 1)
        {
            data[1] *= gain;
        }
        else
        {
            data[2 * k] *= gain;
            data[2 * k + 1] *= gain;
        }
    }
    fft.performRealOnlyInverseTransform (data);
    window.multiplyWithWindowingTable (data, fftSize);
    for (int i = 0; i < fftSize; ++i)
        output[(outputIndex + i) % fftSize] += data[i] / (float) oversampling;
}

void FormantShifterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = juce::jmin (2, getTotalNumInputChannels());
    shiftSmoothed.setTargetValue (readParameter ("formantShift"));
    envelopeSmoothed.setTargetValue (readParameter ("envelope"));
    mixSmoothed.setTargetValue (readParameter ("dryWet") / 100.0f);
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        shiftSmoothed.getNextValue();
        envelopeSmoothed.getNextValue();
        const float mix = mixSmoothed.getNextValue();
        for (int ch = 0; ch < channels; ++ch)
        {
            const float dry = buffer.getSample (ch, sample);
            inputFifo.setSample (ch, fifoIndex, dry);
            const float wet = outputFifo.getSample (ch, outputIndex);
            outputFifo.setSample (ch, outputIndex, 0.0f);
            float out = dry * (1.0f - mix) + wet * mix;
            if (parameters.getRawParameterValue ("limiter")->load() > 0.5f)
                out = std::tanh (out);
            buffer.setSample (ch, sample, out);
        }
        ++fifoIndex;
        outputIndex = (outputIndex + 1) % fftSize;
        if (fifoIndex == fftSize)
        {
            for (int ch = 0; ch < channels; ++ch) processFrame (ch);
            std::memmove (inputFifo.getWritePointer (0),
                          inputFifo.getReadPointer (0) + hopSize,
                          (size_t) (fftSize - hopSize) * sizeof (float));
            if (channels > 1)
                std::memmove (inputFifo.getWritePointer (1),
                              inputFifo.getReadPointer (1) + hopSize,
                              (size_t) (fftSize - hopSize) * sizeof (float));
            fifoIndex = fftSize - hopSize;
        }
    }
    for (int ch = channels; ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());
}

juce::AudioProcessorEditor* FormantShifterAudioProcessor::createEditor()
{
    return new FormantShifterAudioProcessorEditor (*this);
}

void FormantShifterAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, dest);
}

void FormantShifterAudioProcessor::setStateInformation (const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, size));
    if (xml != nullptr && xml->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FormantShifterAudioProcessor();
}
