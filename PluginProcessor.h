#pragma once

#include <JuceHeader.h>

class FormantShifterAudioProcessor final : public juce::AudioProcessor
{
public:
    FormantShifterAudioProcessor();

    void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Formant Shifter"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.1; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }

private:
    static constexpr int fftOrder = 11;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int oversampling = 4;
    static constexpr int hopSize = fftSize / oversampling;

    juce::AudioProcessorValueTreeState parameters;
    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { fftSize, juce::dsp::WindowingFunction<float>::hann };
    juce::AudioBuffer<float> inputFifo, outputFifo;
    juce::AudioBuffer<float> fftData;
    int fifoIndex = 0;
    int outputIndex = 0;
    double currentSampleRate = 44100.0;
    std::vector<float> envelope, shiftedEnvelope;
    std::vector<float> frameMagnitudes, spectralEnvelope;
    juce::SmoothedValue<float> shiftSmoothed, envelopeSmoothed, mixSmoothed;

    void processFrame (int channel);
    float readParameter (const char* id) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FormantShifterAudioProcessor)
};
