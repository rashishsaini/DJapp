#pragma once
#include <JuceHeader.h>

// ═══════════════════════════════════════════════════════════════════
//  Custom look & feel — draws dark DJ-style rotary knobs
// ═══════════════════════════════════════════════════════════════════
class DJKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    explicit DJKnobLookAndFeel(juce::Colour accentColour)
        : accent(accentColour) {
    }

    void drawRotarySlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPos,
        float startAngle, float endAngle,
        juce::Slider& slider) override;
private:
    juce::Colour accent;
};

// ═══════════════════════════════════════════════════════════════════
//  One knob + its band name label (e.g. "LOW")
// ═══════════════════════════════════════════════════════════════════
class EQKnobUnit : public juce::Component
{
public:
    explicit EQKnobUnit(const juce::String& bandName);
    ~EQKnobUnit() override = default;

    void resized() override;

    juce::Slider slider;
    juce::Label  label;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQKnobUnit)
};

// ═══════════════════════════════════════════════════════════════════
//  Full EQ panel for one deck (three knobs + panel paint)
// ═══════════════════════════════════════════════════════════════════
class DeckEQComponent : public juce::Component,
    private juce::Slider::Listener
{
public:
    DeckEQComponent(const juce::String& deckName, juce::Colour deckColour);
    ~DeckEQComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Set these before adding to the UI — they fire whenever a knob moves
    std::function<void(float)> onLowChanged;
    std::function<void(float)> onMidChanged;
    std::function<void(float)> onHighChanged;

private:
    void sliderValueChanged(juce::Slider* s) override;

    const juce::String name;
    const juce::Colour colour;

    DJKnobLookAndFeel laf;       // ← MUST be declared before the knob units
    EQKnobUnit        lowUnit;
    EQKnobUnit        midUnit;
    EQKnobUnit        highUnit;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckEQComponent)
};