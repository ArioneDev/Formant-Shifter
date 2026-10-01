cmake_minimum_required(VERSION 3.22)
project(FormantShifter VERSION 1.1.0 LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    juce
    URL https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.8.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(juce)

juce_add_plugin(FormantShifter
    COMPANY_NAME "Shifter"
    BUNDLE_ID "com.shifter.formantshifter.v110"
    PLUGIN_MANUFACTURER_CODE Shft
    PLUGIN_CODE FmS2
    FORMATS VST3
    PRODUCT_NAME "Formant Shifter 1.1.0"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)
juce_generate_juce_header(FormantShifter)

target_sources(FormantShifter PRIVATE
    src/PluginProcessor.cpp
    src/PluginProcessor.h
    src/PluginEditor.cpp
    src/PluginEditor.h
)

target_compile_features(FormantShifter PRIVATE cxx_std_17)
target_compile_definitions(FormantShifter PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
target_link_libraries(FormantShifter PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
