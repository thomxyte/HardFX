#pragma once

#include <JuceHeader.h>

//==============================================================================
class HardFXAudioProcessor : public juce::AudioProcessor,
                              public juce::AudioProcessorValueTreeState::Listener
{
public:
    HardFXAudioProcessor();
    ~HardFXAudioProcessor() override;

    //==========================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==========================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==========================================================================
    const juce::String getName() const override { return JucePlugin_Name; }
    bool   acceptsMidi()  const override { return false; }
    bool   producesMidi() const override { return false; }
    bool   isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==========================================================================
    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==========================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    double sr = 44100.0;

    // --- Distortion ---
    std::atomic<bool>  distOn   { true };
    std::atomic<float> distDrive{ 5.0f };
    std::atomic<float> distMix  { 1.0f };

    // --- Saturator ---
    std::atomic<bool>  satOn     { true };
    std::atomic<float> satAmount { 0.5f };
    std::atomic<float> satMix    { 1.0f };

    // --- Comb Filter ---
    std::atomic<bool>  combOn  { true };
    std::atomic<float> combHz  { 400.0f };
    std::atomic<float> combFB  { 0.5f };
    std::atomic<float> combMix { 0.5f };

    static constexpr int COMB_SIZE = 8192;
    float combBufL[COMB_SIZE] = {};
    float combBufR[COMB_SIZE] = {};
    int   combWrite = 0;

    // --- Growl Filter ---
    std::atomic<bool>  growlOn    { true };
    std::atomic<float> growlRate  { 2.0f };
    std::atomic<float> growlDepth { 0.5f };
    std::atomic<float> growlMix   { 0.5f };

    double growlPhase = 0.0;

    juce::IIRFilter f1L, f1R, f2L, f2R;

    void updateFormants (float lfo);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HardFXAudioProcessor)
};
