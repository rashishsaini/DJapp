#include "CueMixerComponent.h"

CueMixerComponent::CueMixerComponent(CueMixer& processor)
	: cueMixer(processor)
{
    // ── Cue section label ─────────────────────────────────────────
    cueSectionLabel.setText("CUE  /  HEADPHONE MONITOR",
        juce::dontSendNotification);
    cueSectionLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    cueSectionLabel.setJustificationType(juce::Justification::centred);
    cueSectionLabel.setColour(juce::Label::textColourId,
        juce::Colour(0xff22bb88));
    addAndMakeVisible(cueSectionLabel);

    // ── CUE toggle buttons ────────────────────────────────────────
    // setClickingTogglesState keeps the button latched when pressed.
    auto setupCueButton = [&](juce::TextButton& btn,
        juce::Colour accent,
        std::function<void(bool)> onToggle)
        {
            btn.setClickingTogglesState(true);
            btn.setToggleState(false, juce::dontSendNotification);
            btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff111128));
            btn.setColour(juce::TextButton::buttonOnColourId, accent.withAlpha(0.85f));
            btn.setColour(juce::TextButton::textColourOffId, accent.withAlpha(0.55f));
            btn.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
            btn.onClick = [onToggle, &btn] { onToggle(btn.getToggleState()); };
            addAndMakeVisible(btn);
        };

    setupCueButton(deck1CueBtn, juce::Colour(0xff00aaff),
        [this](bool on) { cueMixer.setDeck1Cue(on); });

    setupCueButton(deck2CueBtn, juce::Colour(0xffff5500),
        [this](bool on) { cueMixer.setDeck2Cue(on); });

    // ── Headphone gain slider ─────────────────────────────────────
    cueGainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    cueGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    cueGainSlider.setRange(0.0, 2.0);          // 0 = mute … 2.0 ≈ +6 dB
    cueGainSlider.setValue(1.0, juce::dontSendNotification);
    cueGainSlider.setDoubleClickReturnValue(true, 1.0); // double-click → unity

    cueGainSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff0a1a12));
    cueGainSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff1a4a30));
    cueGainSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff22bb88));

    cueGainSlider.onValueChange = [this]
        {
            cueMixer.setCueGain(static_cast<float>(cueGainSlider.getValue()));
        };
    addAndMakeVisible(cueGainSlider);

    cueGainLabel.setText("GAIN", juce::dontSendNotification);
    cueGainLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    cueGainLabel.setJustificationType(juce::Justification::centred);
    cueGainLabel.setColour(juce::Label::textColourId, juce::Colour(0xff22bb88).withAlpha(0.7f));
    addAndMakeVisible(cueGainLabel);

    setSize(720, 460);
}

void CueMixerComponent::resized()
{
    auto area = getLocalBounds().reduced(8);

    // Section title
    cueSectionLabel.setBounds(area.removeFromTop(18));

    area.removeFromTop(4);

    auto controlRow = area;

    // Cue buttons
    deck1CueBtn.setBounds(
        controlRow.removeFromLeft(80).reduced(4, 6));

    deck2CueBtn.setBounds(
        controlRow.removeFromRight(80).reduced(4, 6));

    // Gain controls
    auto gainArea = controlRow.reduced(10, 0);

    cueGainLabel.setBounds(
        gainArea.removeFromTop(15));

    cueGainSlider.setBounds(gainArea);
}

void CueMixerComponent::paint(juce::Graphics& g)
{
    // Background panel
    auto panel = getLocalBounds().toFloat();

    g.setColour(juce::Colour(0xff071510));
    g.fillRoundedRectangle(panel, 10.0f);

    g.setColour(juce::Colour(0xff1a4a30));
    g.drawRoundedRectangle(panel.reduced(0.5f), 10.0f, 1.0f);
}