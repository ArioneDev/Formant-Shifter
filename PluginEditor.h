#pragma once

#include "PluginProcessor.h"

class FormantShifterAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit FormantShifterAudioProcessorEditor (FormantShifterAudioProcessor&);
    ~FormantShifterAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class MonoRotaryLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        void drawRotarySlider (juce::Graphics&, int, int, int, int, float,
                               float, float, juce::Slider&) override;
    };

    FormantShifterAudioProcessor& processor;
    MonoRotaryLookAndFeel rotaryLookAndFeel;
    juce::HyperlinkButton versionLink;
    juce::ComboBox modeSelector;
    juce::Label modeLabel;
    juce::Slider formantShiftSlider, envelopeSlider, dryWetSlider;
    juce::Label formantShiftLabel, envelopeLabel, dryWetLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> formantShiftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> envelopeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryWetAttachment;

    void configureSlider (juce::Slider&, juce::Label&, const juce::String&);
    void updateModeLabels();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FormantShifterAudioProcessorEditor)
};
