#include "PluginEditor.h"

void FormantShifterAudioProcessorEditor::MonoRotaryLookAndFeel::drawRotarySlider (
    juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
    float rotaryStartAngle, float rotaryEndAngle, juce::Slider&)
{
    const float size = (float) juce::jmin (width, height) - 10.0f;
    const float cx = (float) x + (float) width * 0.5f;
    const float cy = (float) y + (float) height * 0.5f;
    const float radius = size * 0.5f;
    const juce::Rectangle<float> arcBounds (cx - radius, cy - radius, size, size);
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    g.setColour (juce::Colours::white);
    g.fillEllipse (arcBounds);
    g.setColour (juce::Colours::black);
    juce::Path arc;
    arc.addArc (arcBounds.getX(), arcBounds.getY(), arcBounds.getWidth(), arcBounds.getHeight(),
                rotaryStartAngle, angle, true);
    g.strokePath (arc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));

    const auto arcEnd = arc.getCurrentPosition();
    const auto direction = arcEnd - juce::Point<float> (cx, cy);
    const float directionLength = juce::jmax (direction.getDistanceFromOrigin(), 1.0f);
    const auto indicatorEnd = juce::Point<float> (cx, cy)
                            + direction * ((radius - 10.0f) / directionLength);
    g.drawLine (cx, cy, indicatorEnd.x, indicatorEnd.y, 3.0f);
    g.fillEllipse (juce::Rectangle<float> (cx - 3.0f, cy - 3.0f, 6.0f, 6.0f));
}

FormantShifterAudioProcessorEditor::FormantShifterAudioProcessorEditor (
    FormantShifterAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (560, 340);
    setResizable (true, false);

    configureSlider (formantShiftSlider, formantShiftLabel, "FORMANT SHIFT");
    configureSlider (envelopeSlider, envelopeLabel, "ENVELOPE");
    configureSlider (dryWetSlider, dryWetLabel, "DRY / WET");

    formantShiftSlider.setRange (-1200.0, 1200.0, 1.0);
    formantShiftSlider.setNumDecimalPlacesToDisplay (0);
    formantShiftSlider.setTextValueSuffix (" ct");
    envelopeSlider.setRange (0.0, 16.0, 0.01);
    envelopeSlider.setNumDecimalPlacesToDisplay (1);
    dryWetSlider.setRange (0.0, 100.0, 0.01);
    dryWetSlider.setNumDecimalPlacesToDisplay (1);
    dryWetSlider.setTextValueSuffix (" %");

    auto& state = processor.getParameters();
    formantShiftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (state, "formantShift", formantShiftSlider);
    envelopeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (state, "envelope", envelopeSlider);
    dryWetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (state, "dryWet", dryWetSlider);
}

void FormantShifterAudioProcessorEditor::configureSlider (
    juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                juce::MathConstants<float>::pi * 2.75f, true);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 20);
    slider.setLookAndFeel (&rotaryLookAndFeel);
    slider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::black);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::white);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::black);
    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setFont (juce::Font (juce::FontOptions{}.withHeight (12.0f).withStyle ("Bold")));
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, juce::Colours::black);
    addAndMakeVisible (label);
}

void FormantShifterAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::white);
    g.setColour (juce::Colours::black);
    g.drawRect (getLocalBounds().toFloat().reduced (2.0f), 2.0f);
    g.drawLine (28.0f, 34.0f, 28.0f, 82.0f, 2.0f);

    g.setFont (juce::Font (juce::FontOptions{}.withHeight (24.0f)));
    g.drawText ("Formant Shifter", 44, 25, 300, 30, juce::Justification::left);
    g.setFont (juce::Font (juce::FontOptions{}.withHeight (14.0f)));
    g.drawText ("v1.0.1", getWidth() - 92, 30, 64, 22, juce::Justification::right);
    g.setFont (juce::Font (juce::FontOptions{}.withHeight (13.0f)));
    g.drawText ("Made By Arione", 45, 56, 220, 20, juce::Justification::left);
}

void FormantShifterAudioProcessorEditor::resized()
{
    const int top = 92;
    const int columnWidth = getWidth() / 3;
    auto place = [top, columnWidth] (juce::Slider& slider, juce::Label& label, int column)
    {
        slider.setBounds (column * columnWidth + 24, top, columnWidth - 48, 190);
        label.setBounds (column * columnWidth + 10, top + 202, columnWidth - 20, 24);
    };
    place (formantShiftSlider, formantShiftLabel, 0);
    place (envelopeSlider, envelopeLabel, 1);
    place (dryWetSlider, dryWetLabel, 2);
}
