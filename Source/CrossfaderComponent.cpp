#include "CrossfaderComponent.h"

CrossfaderComponent::CrossfaderComponent(Crossfader& processor)
    : crossfader(processor)
{
    crossfaderSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    crossfaderSlider.setTextBoxStyle(
        juce::Slider::NoTextBox,
        false,
        0,
        0);

    crossfaderSlider.setRange(0.0, 1.0);

    crossfaderSlider.setValue(
        0.5,
        juce::dontSendNotification);

    crossfaderSlider.setDoubleClickReturnValue(
        true,
        0.5);

    crossfaderSlider.setColour(
        juce::Slider::backgroundColourId,
        juce::Colour(0xff111128));

    crossfaderSlider.setColour(
        juce::Slider::trackColourId,
        juce::Colour(0xff2a2a4a));

    crossfaderSlider.setColour(
        juce::Slider::thumbColourId,
        juce::Colours::white);

    crossfaderSlider.onValueChange =
        [this]
        {
            crossfader.setPosition(
                (float)crossfaderSlider.getValue());
        };

    addAndMakeVisible(crossfaderSlider);

    auto setupLabel =
        [&](juce::Label& lbl,
            const juce::String& text,
            juce::Justification just,
            juce::Colour colour)
        {
            lbl.setText(
                text,
                juce::dontSendNotification);

            lbl.setFont(
                juce::Font(
                    juce::FontOptions(12.0f,
                        juce::Font::bold)));

            lbl.setJustificationType(just);

            lbl.setColour(
                juce::Label::textColourId,
                colour);

            addAndMakeVisible(lbl);
        };

    setupLabel(
        crossfaderTitle,
        "CROSSFADER",
        juce::Justification::centred,
        juce::Colour(0xff888899));

    setupLabel(
        deck1XfadeLabel,
        "DECK 1",
        juce::Justification::centredLeft,
        juce::Colour(0xff00aaff));

    setupLabel(
        deck2XfadeLabel,
        "DECK 2",
        juce::Justification::centredRight,
        juce::Colour(0xffff5500));
}

void CrossfaderComponent::resized()
{
    auto area = getLocalBounds().reduced(10);

    crossfaderTitle.setBounds(
        area.removeFromTop(24));

    area.removeFromTop(6);

    auto labelRow = area.removeFromTop(20);

    deck1XfadeLabel.setBounds(
        labelRow.removeFromLeft(120));

    deck2XfadeLabel.setBounds(
        labelRow.removeFromRight(120));

    area.removeFromTop(8);

    crossfaderSlider.setBounds(
        area.removeFromTop(30));
}

void CrossfaderComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff1b1b28));

    g.fillRoundedRectangle(
        getLocalBounds().toFloat(),
        8.0f);
}