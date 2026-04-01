#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class HardFXLookAndFeel : public juce::LookAndFeel_V4
{
public:
    HardFXLookAndFeel()
    {
        setColour (juce::Slider::thumbColourId,         juce::Colour (0xFFFF4500));
        setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xFFFF4500));
        setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xFF333333));
        setColour (juce::Slider::textBoxTextColourId,   juce::Colour (0xFFAAAAAA));
        setColour (juce::Slider::textBoxOutlineColourId,juce::Colour (0xFF333333));
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xFF1A1A1A));
        setColour (juce::ToggleButton::tickColourId,    juce::Colour (0xFFFF4500));
        setColour (juce::ToggleButton::tickDisabledColourId, juce::Colour (0xFF444444));
        setColour (juce::Label::textColourId,           juce::Colour (0xFF999999));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider& /*slider*/) override
    {
        const float cx = x + width  * 0.5f;
        const float cy = y + height * 0.5f;
        const float r  = juce::jmin (width, height) * 0.38f;
        const float angle = startAngle + sliderPos * (endAngle - startAngle);
        const float ar = r * 0.82f;

        // Background
        g.setColour (juce::Colour (0xFF222222));
        g.fillEllipse (cx-r, cy-r, r*2.0f, r*2.0f);

        // Track
        juce::Path track;
        track.addArc (cx-ar, cy-ar, ar*2.0f, ar*2.0f, startAngle, endAngle, true);
        g.setColour (juce::Colour (0xFF383838));
        g.strokePath (track, juce::PathStrokeType (2.5f));

        // Fill
        juce::Path fill;
        fill.addArc (cx-ar, cy-ar, ar*2.0f, ar*2.0f, startAngle, angle, true);
        g.setColour (juce::Colour (0xFFFF4500));
        g.strokePath (fill, juce::PathStrokeType (2.5f));

        // Pointer
        juce::Path ptr;
        ptr.startNewSubPath (0.0f, -r*0.55f);
        ptr.lineTo (0.0f, -r*0.9f);
        g.setColour (juce::Colour (0xFFFF6622));
        g.strokePath (ptr, juce::PathStrokeType (2.0f),
                      juce::AffineTransform::rotation (angle).translated (cx, cy));
    }
};

//==============================================================================
class LabelledKnob : public juce::Component
{
public:
    juce::Slider slider;
    juce::Label  label;

    explicit LabelledKnob (const juce::String& labelText)
    {
        slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 13);
        addAndMakeVisible (slider);

        label.setText (labelText, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::Font (juce::FontOptions().withHeight (10.0f)));
        addAndMakeVisible (label);
    }

    void resized() override
    {
        auto b = getLocalBounds();
        label.setBounds (b.removeFromBottom (14));
        slider.setBounds (b);
    }
};

//==============================================================================
class HardFXAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit HardFXAudioProcessorEditor (HardFXAudioProcessor&);
    ~HardFXAudioProcessorEditor() override;

    void paint  (juce::Graphics&) override;
    void resized() override;

private:
    HardFXAudioProcessor& proc;
    HardFXLookAndFeel     laf;

    // Toggles
    juce::ToggleButton distToggle  { "ON" };
    juce::ToggleButton satToggle   { "ON" };
    juce::ToggleButton combToggle  { "ON" };
    juce::ToggleButton growlToggle { "ON" };

    // Knobs
    LabelledKnob kDistDrive  { "DRIVE"    };
    LabelledKnob kDistMix    { "MIX"      };
    LabelledKnob kSatAmount  { "AMOUNT"   };
    LabelledKnob kSatMix     { "MIX"      };
    LabelledKnob kCombHz     { "FREQ"     };
    LabelledKnob kCombFB     { "FEEDBACK" };
    LabelledKnob kCombMix    { "MIX"      };
    LabelledKnob kGrowlRate  { "RATE"     };
    LabelledKnob kGrowlDepth { "DEPTH"    };
    LabelledKnob kGrowlMix   { "MIX"      };

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<BA> baDist, baSat, baComb, baGrowl;
    std::unique_ptr<SA> saDrive, saDMix;
    std::unique_ptr<SA> saAmt,   saSMix;
    std::unique_ptr<SA> saHz,    saFB,  saCMix;
    std::unique_ptr<SA> saRate,  saDep, saGMix;

    struct ColourDef { juce::Colour colour; const char* title; };
    static constexpr ColourDef COLS[4] = {
        { juce::Colour (0xFFFF4500), "DISTORTION"  },
        { juce::Colour (0xFFFFAA00), "SATURATOR"   },
        { juce::Colour (0xFF00AAFF), "COMB FILTER" },
        { juce::Colour (0xFF00FF88), "GROWL FILTER"}
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HardFXAudioProcessorEditor)
};
