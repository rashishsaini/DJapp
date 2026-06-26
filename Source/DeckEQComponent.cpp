#include "DeckEQComponent.h"

// ───────────────────────────────────────────────────────────────────
//  DJKnobLookAndFeel — custom knob painter
// ───────────────────────────────────────────────────────────────────
void DJKnobLookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y, int w, int h,
    float sliderPos,
    float startAngle, float endAngle,
    juce::Slider& /*slider*/)
{
    const float radius = juce::jmin(w, h) * 0.42f;
    const float cx = (float)x + w * 0.5f;
    const float cy = (float)y + h * 0.5f;
    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    // 1. Subtle glow behind the knob
    g.setColour(accent.withAlpha(0.18f));
    g.fillEllipse(cx - radius - 5, cy - radius - 5,
        (radius + 5) * 2.0f, (radius + 5) * 2.0f);

    // 2. Dark body
    g.setColour(juce::Colour(0xff16213e));
    g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

    // 3. Thin border
    g.setColour(juce::Colour(0xff2d2d55));
    g.drawEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 1.5f);

    // 4. Coloured arc from start angle to current position
    const float arcR = radius - 5.0f;
    juce::Path arc;
    arc.addCentredArc(cx, cy, arcR, arcR, 0.0f, startAngle, angle, true);
    g.setColour(accent);
    g.strokePath(arc, juce::PathStrokeType(4.0f,
        juce::PathStrokeType::curved,
        juce::PathStrokeType::rounded));

    // 5. White pointer line
    juce::Path ptr;
    const float pLen = radius * 0.55f;
    ptr.addRectangle(-1.5f, -pLen, 3.0f, pLen);
    ptr.applyTransform(juce::AffineTransform::rotation(angle)
        .translated(cx, cy));
    g.setColour(juce::Colours::white);
    g.fillPath(ptr);

    // 6. Accent dot at centre
    g.setColour(accent.brighter(0.4f));
    g.fillEllipse(cx - 3.5f, cy - 3.5f, 7.0f, 7.0f);
}

// ───────────────────────────────────────────────────────────────────
//  EQKnobUnit
// ───────────────────────────────────────────────────────────────────
EQKnobUnit::EQKnobUnit(const juce::String& bandName)
{
    // Rotary-drag style (move mouse up/down to turn the knob)
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 58, 18);
    slider.setRange(0.0, 1.0);
    slider.setValue(0.5, juce::dontSendNotification);
    slider.setDoubleClickReturnValue(true, 0.5);   // double-click resets to 0.5
    slider.setNumDecimalPlacesToDisplay(3);
    addAndMakeVisible(slider);

    label.setText(bandName, juce::dontSendNotification);
    label.setFont(juce::Font(13.0f, juce::Font::bold));
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, juce::Colour(0xffaaaacc));
    addAndMakeVisible(label);
}

void EQKnobUnit::resized()
{
    auto b = getLocalBounds();
    label.setBounds(b.removeFromTop(20));   // band name at top
    slider.setBounds(b.reduced(4));          // dial + text box fill the rest
}

// ───────────────────────────────────────────────────────────────────
//  DeckEQComponent
// ───────────────────────────────────────────────────────────────────
DeckEQComponent::DeckEQComponent(const juce::String& deckName,
    juce::Colour deckColour)
    : name(deckName),
    colour(deckColour),
    laf(deckColour),       // initialise look & feel first
    lowUnit("LOW"),
    midUnit("MID"),
    highUnit("HIGH")
{
    // Apply our custom skin to this panel AND all its children
    setLookAndFeel(&laf);

    addAndMakeVisible(lowUnit);
    addAndMakeVisible(midUnit);
    addAndMakeVisible(highUnit);

    lowUnit.slider.addListener(this);
    midUnit.slider.addListener(this);
    highUnit.slider.addListener(this);
}

DeckEQComponent::~DeckEQComponent()
{
    // Must clear LookAndFeel before it (a member) is destroyed
    setLookAndFeel(nullptr);

    lowUnit.slider.removeListener(this);
    midUnit.slider.removeListener(this);
    highUnit.slider.removeListener(this);
}

void DeckEQComponent::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced(1.0f);

    // Panel background
    g.setColour(juce::Colour(0xff0d0d1f));
    g.fillRoundedRectangle(b, 10.0f);

    // Deck colour border
    g.setColour(colour.withAlpha(0.55f));
    g.drawRoundedRectangle(b, 10.0f, 1.5f);

    // Deck name at the top of the panel
    g.setColour(colour);
    g.setFont(juce::Font(15.0f, juce::Font::bold));
    g.drawText(name,
        getLocalBounds().removeFromTop(28).reduced(4),
        juce::Justification::centred, true);
}

void DeckEQComponent::resized()
{
    auto area = getLocalBounds().reduced(6);
    area.removeFromTop(28);           // leave room for the deck name

    const int kw = area.getWidth() / 3;
    lowUnit.setBounds(area.removeFromLeft(kw).reduced(2));
    midUnit.setBounds(area.removeFromLeft(kw).reduced(2));
    highUnit.setBounds(area.reduced(2));
}

void DeckEQComponent::sliderValueChanged(juce::Slider* s)
{
    const float knob = static_cast<float>(s->getValue());

    float db;

    if (knob <= 0.5f)
    {
        db = juce::jmap(knob,
            0.0f, 0.5f,
            -60.0f, 0.0f);
    }
    else
    {
        db = juce::jmap(knob,
            0.5f, 1.0f,
            0.0f, 12.0f);
    }

    if (s == &lowUnit.slider && onLowChanged)  onLowChanged(db);
    else if (s == &midUnit.slider && onMidChanged)  onMidChanged(db);
    else if (s == &highUnit.slider && onHighChanged) onHighChanged(db);
}