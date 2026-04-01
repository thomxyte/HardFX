#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HardFXAudioProcessor::HardFXAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    const juce::StringArray ids {
        "distOn","distDrive","distMix",
        "satOn","satAmount","satMix",
        "combOn","combHz","combFB","combMix",
        "growlOn","growlRate","growlDepth","growlMix"
    };
    for (auto& id : ids)
        apvts.addParameterListener (id, this);
}

HardFXAudioProcessor::~HardFXAudioProcessor()
{
    const juce::StringArray ids {
        "distOn","distDrive","distMix",
        "satOn","satAmount","satMix",
        "combOn","combHz","combFB","combMix",
        "growlOn","growlRate","growlDepth","growlMix"
    };
    for (auto& id : ids)
        apvts.removeParameterListener (id, this);
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout HardFXAudioProcessor::createParameterLayout()
{
    using P  = juce::AudioParameterFloat;
    using PB = juce::AudioParameterBool;
    using NR = juce::NormalisableRange<float>;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<PB> ("distOn",    "Dist On",      true));
    params.push_back (std::make_unique<P>  ("distDrive",  "Dist Drive",  NR(1.f,40.f,0.1f),  5.f));
    params.push_back (std::make_unique<P>  ("distMix",    "Dist Mix",    NR(0.f,1.f,0.01f),  1.f));

    params.push_back (std::make_unique<PB> ("satOn",     "Sat On",       true));
    params.push_back (std::make_unique<P>  ("satAmount",  "Sat Amount",  NR(0.f,1.f,0.01f),  0.5f));
    params.push_back (std::make_unique<P>  ("satMix",     "Sat Mix",     NR(0.f,1.f,0.01f),  1.f));

    params.push_back (std::make_unique<PB> ("combOn",    "Comb On",      true));
    params.push_back (std::make_unique<P>  ("combHz",     "Comb Freq",   NR(50.f,5000.f,1.f,0.4f), 400.f));
    params.push_back (std::make_unique<P>  ("combFB",     "Comb FB",     NR(-0.97f,0.97f,0.01f),   0.5f));
    params.push_back (std::make_unique<P>  ("combMix",    "Comb Mix",    NR(0.f,1.f,0.01f),  0.5f));

    params.push_back (std::make_unique<PB> ("growlOn",   "Growl On",     true));
    params.push_back (std::make_unique<P>  ("growlRate",  "Growl Rate",  NR(0.1f,20.f,0.01f,0.5f), 2.f));
    params.push_back (std::make_unique<P>  ("growlDepth", "Growl Depth", NR(0.f,1.f,0.01f),  0.5f));
    params.push_back (std::make_unique<P>  ("growlMix",   "Growl Mix",   NR(0.f,1.f,0.01f),  0.5f));

    return { params.begin(), params.end() };
}

void HardFXAudioProcessor::parameterChanged (const juce::String& id, float v)
{
    if      (id == "distOn")    distOn    = (v > 0.5f);
    else if (id == "distDrive") distDrive = v;
    else if (id == "distMix")   distMix   = v;
    else if (id == "satOn")     satOn     = (v > 0.5f);
    else if (id == "satAmount") satAmount = v;
    else if (id == "satMix")    satMix    = v;
    else if (id == "combOn")    combOn    = (v > 0.5f);
    else if (id == "combHz")    combHz    = v;
    else if (id == "combFB")    combFB    = v;
    else if (id == "combMix")   combMix   = v;
    else if (id == "growlOn")   growlOn   = (v > 0.5f);
    else if (id == "growlRate") growlRate = v;
    else if (id == "growlDepth")growlDepth= v;
    else if (id == "growlMix")  growlMix  = v;
}

//==============================================================================
void HardFXAudioProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;
    juce::zeromem (combBufL, sizeof (combBufL));
    juce::zeromem (combBufR, sizeof (combBufR));
    combWrite  = 0;
    growlPhase = 0.0;
    f1L.reset(); f1R.reset();
    f2L.reset(); f2R.reset();
}

void HardFXAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HardFXAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}
#endif

//==============================================================================
void HardFXAudioProcessor::updateFormants (float lfo)
{
    float t   = (lfo + 1.0f) * 0.5f;
    float ff1 = 400.0f + t * 400.0f;
    float ff2 = 800.0f + t * 1400.0f;
    auto c1 = juce::IIRCoefficients::makeBandPass ((float)sr, ff1, 3.0f);
    auto c2 = juce::IIRCoefficients::makeBandPass ((float)sr, ff2, 3.0f);
    f1L.setCoefficients (c1);  f1R.setCoefficients (c1);
    f2L.setCoefficients (c2);  f2R.setCoefficients (c2);
}

void HardFXAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                         juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numCh      = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    float* L = buffer.getWritePointer (0);
    float* R = (numCh > 1) ? buffer.getWritePointer (1) : nullptr;

    // Snapshot atomics once per block for efficiency
    const bool  dOn  = distOn.load();
    const float dDrv = distDrive.load();
    const float dMix = distMix.load();

    const bool  sOn  = satOn.load();
    const float sAmt = satAmount.load();
    const float sMix = satMix.load();

    const bool  cOn  = combOn.load();
    const float cHz  = combHz.load();
    const float cFB  = combFB.load();
    const float cMix = combMix.load();

    const bool  gOn  = growlOn.load();
    const float gRt  = growlRate.load();
    const float gDep = growlDepth.load();
    const float gMix = growlMix.load();

    const int combDelay = juce::jlimit (1, COMB_SIZE - 1, (int)(sr / (double)cHz));

    for (int i = 0; i < numSamples; ++i)
    {
        float l = L[i];
        float r = R ? R[i] : l;

        // ── DISTORTION ────────────────────────────────────────────────────
        if (dOn)
        {
            const float denom = std::tanh (dDrv);
            if (denom > 1e-6f)
            {
                const float dl = std::tanh (l * dDrv) / denom;
                const float dr = std::tanh (r * dDrv) / denom;
                l = dMix * dl + (1.0f - dMix) * l;
                r = dMix * dr + (1.0f - dMix) * r;
            }
        }

        // ── SATURATOR ─────────────────────────────────────────────────────
        if (sOn)
        {
            const float g    = 1.0f + sAmt * 4.0f;
            const float norm = g / (1.0f + g);   // softclip(g) for normalisation
            const float sl   = (l * g / (1.0f + std::abs (l * g))) / norm;
            const float sr2  = (r * g / (1.0f + std::abs (r * g))) / norm;
            l = sMix * sl + (1.0f - sMix) * l;
            r = sMix * sr2 + (1.0f - sMix) * r;
        }

        // ── COMB FILTER ───────────────────────────────────────────────────
        if (cOn)
        {
            const int rp = (combWrite - combDelay + COMB_SIZE) % COMB_SIZE;
            const float dL = combBufL[rp];
            const float dR = combBufR[rp];
            combBufL[combWrite] = l + cFB * dL;
            combBufR[combWrite] = r + cFB * dR;
            l = cMix * dL + (1.0f - cMix) * l;
            r = cMix * dR + (1.0f - cMix) * r;
            combWrite = (combWrite + 1) % COMB_SIZE;
        }

        // ── GROWL FILTER ──────────────────────────────────────────────────
        if (gOn)
        {
            const float lfo = (float)std::sin (growlPhase) * gDep;
            updateFormants (lfo);
            const float gl = f1L.processSingleSampleRaw (l) + f2L.processSingleSampleRaw (l);
            const float gr = f1R.processSingleSampleRaw (r) + f2R.processSingleSampleRaw (r);
            l = gMix * gl + (1.0f - gMix) * l;
            r = gMix * gr + (1.0f - gMix) * r;
            growlPhase += juce::MathConstants<double>::twoPi * (double)gRt / sr;
            if (growlPhase >= juce::MathConstants<double>::twoPi)
                growlPhase -= juce::MathConstants<double>::twoPi;
        }

        L[i] = l;
        if (R) R[i] = r;
    }
}

//==============================================================================
void HardFXAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void HardFXAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HardFXAudioProcessor();
}
