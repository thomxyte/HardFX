#include "PluginProcessor.h"
#include "PluginEditor.h"

// Static member definition
constexpr HardFXAudioProcessorEditor::ColourDef HardFXAudioProcessorEditor::COLS[4];

//==============================================================================
HardFXAudioProcessorEditor::HardFXAudioProcessorEditor (HardFXAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&laf);
    setSize (700, 300);
    setResizable (false, false);

    // Make everything visible
    for (auto* w : { &distToggle, &satToggle, &combToggle, &growlToggle })
        addAndMakeVisible (w);

    for (auto* k : { &kDistDrive, &kDistMix, &kSatAmount, &kSatMix,
                     &kCombHz, &kCombFB, &kCombMix,
                     &kGrowlRate, &kGrowlDepth, &kGrowlMix })
        addAndMakeVisible (k);

    // Attach parameters
    auto& a = proc.apvts;
    baDist  = std::make_unique<BA> (a, "distOn",    distToggle);
    baSat   = std::make_unique<BA> (a, "satOn",     satToggle);
    baComb  = std::make_unique<BA> (a, "combOn",    combToggle);
    baGrowl = std::make_unique<BA> (a, "growlOn",   growlToggle);

    saDrive = std::make_unique<SA> (a, "distDrive", kDistDrive.slider);
    saDMix  = std::make_unique<SA> (a, "distMix",   kDistMix.slider);
    saAmt   = std::make_unique<SA> (a, "satAmount", kSatAmount.slider);
    saSMix  = std::make_unique<SA> (a, "satMix",    kSatMix.slider);
    saHz    = std::make_unique<SA> (a, "combHz",    kCombHz.slider);
    saFB    = std::make_unique<SA> (a, "combFB",    kCombFB.slider);
    saCMix  = std::make_unique<SA> (a, "combMix",   kCombMix.slider);
    saRate  = std::make_unique<SA> (a, "growlRate", kGrowlRate.slider);
    saDep   = std::make_unique<SA> (a, "growlDepth",kGrowlDepth.slider);
    saGMix  = std::make_unique<SA> (a, "growlMix",  kGrowlMix.slider);
}

HardFXAudioProcessorEditor::~HardFXAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void HardFXAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF0F0F0F));

    // Title bar
    g.setColour (juce::Colour (0xFF1A1A1A));
    g.fillRect (0, 0, getWidth(), 28);
    g.setColour (juce::Colour (0xFFFF4500));
    g.setFont (juce::Font (juce::FontOptions().withHeight (16.0f).withStyle ("Bold")));
    g.drawText ("HARDFX", 10, 0, 120, 28, juce::Justification::centredLeft);
    g.setColour (juce::Colour (0xFF555555));
    g.setFont (juce::Font (juce::FontOptions().withHeight (10.0f)));
    g.drawText ("by Thom Xyte", getWidth()-100, 0, 90, 28, juce::Justification::centredRight);

    // Module panels
    const int mw = getWidth() / 4;
    for (int i = 0; i < 4; ++i)
    {
        auto b = juce::Rectangle<float> ((float)(i*mw + 4), 32.f,
                                         (float)(mw - 8),
                                         (float)(getHeight() - 36));
        g.setColour (juce::Colour (0xFF181818));
        g.fillRoundedRectangle (b, 7.0f);
        g.setColour (COLS[i].colour.withAlpha (0.55f));
        g.drawRoundedRectangle (b, 7.0f, 1.2f);

        g.setColour (COLS[i].colour);
        g.setFont (juce::Font (juce::FontOptions().withHeight (10.0f).withStyle ("Bold")));
        g.drawText (COLS[i].title,
                    (int)b.getX() + 8, (int)b.getY() + 6,
                    mw - 16, 14,
                    juce::Justification::topLeft);
    }
}

void HardFXAudioProcessorEditor::resized()
{
    const int mw     = getWidth() / 4;
    const int topY   = 54;
    const int knobH  = 84;
    const int togH   = 20;
    const int pad    = 4;

    auto col = [&] (int i) -> juce::Rectangle<int>
    {
        return { i*mw + 8, topY, mw - 16, getHeight() - topY - 8 };
    };

    // DISTORTION  (2 knobs)
    {
        auto c = col (0);
        distToggle.setBounds (c.removeFromTop (togH));
        c.removeFromTop (pad);
        int kw = c.getWidth() / 2;
        kDistDrive.setBounds (c.removeFromLeft (kw).withHeight (knobH));
        kDistMix  .setBounds (c.withHeight (knobH));
    }

    // SATURATOR  (2 knobs)
    {
        auto c = col (1);
        satToggle.setBounds (c.removeFromTop (togH));
        c.removeFromTop (pad);
        int kw = c.getWidth() / 2;
        kSatAmount.setBounds (c.removeFromLeft (kw).withHeight (knobH));
        kSatMix   .setBounds (c.withHeight (knobH));
    }

    // COMB FILTER  (3 knobs)
    {
        auto c = col (2);
        combToggle.setBounds (c.removeFromTop (togH));
        c.removeFromTop (pad);
        int kw = c.getWidth() / 3;
        kCombHz .setBounds (c.removeFromLeft (kw).withHeight (knobH));
        kCombFB .setBounds (c.removeFromLeft (kw).withHeight (knobH));
        kCombMix.setBounds (c.withHeight (knobH));
    }

    // GROWL FILTER  (3 knobs)
    {
        auto c = col (3);
        growlToggle.setBounds (c.removeFromTop (togH));
        c.removeFromTop (pad);
        int kw = c.getWidth() / 3;
        kGrowlRate .setBounds (c.removeFromLeft (kw).withHeight (knobH));
        kGrowlDepth.setBounds (c.removeFromLeft (kw).withHeight (knobH));
        kGrowlMix  .setBounds (c.withHeight (knobH));
    }
}
