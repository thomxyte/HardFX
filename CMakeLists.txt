cmake_minimum_required(VERSION 3.22)
project(HardFX VERSION 1.0.0)

# ── JUCE (clonato nella cartella JUCE/ dal workflow) ─────────────────────────
add_subdirectory(JUCE)

# ── Plugin target ─────────────────────────────────────────────────────────────
juce_add_plugin(HardFX
    COMPANY_NAME                "ThomXyte"
    PLUGIN_MANUFACTURER_CODE    TXYT
    PLUGIN_CODE                 HFX1
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "HardFX"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
)

# ── Genera JuceHeader.h (obbligatorio con #include <JuceHeader.h>) ────────────
juce_generate_juce_header(HardFX)

# ── Sorgenti ──────────────────────────────────────────────────────────────────
target_sources(HardFX PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

# ── Defines ───────────────────────────────────────────────────────────────────
target_compile_definitions(HardFX
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
    PRIVATE
        JucePlugin_Name="HardFX"
        JucePlugin_Manufacturer="ThomXyte"
        JucePlugin_ManufacturerCode=0x54585954
        JucePlugin_PluginCode=0x48465831
        JucePlugin_IsSynth=0
        JucePlugin_WantsMidiInput=0
        JucePlugin_ProducesMidiOutput=0
        JucePlugin_IsMidiEffect=0
        JucePlugin_EditorRequiresKeyboardFocus=0
        JucePlugin_Version=1.0.0
        JucePlugin_VersionCode=0x10000
        JucePlugin_VersionString="1.0.0"
)

# ── Link ──────────────────────────────────────────────────────────────────────
target_link_libraries(HardFX
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
    PUBLIC
        juce::juce_recommended_config_flags
)
