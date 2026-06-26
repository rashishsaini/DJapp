#pragma once

#include <JuceHeader.h>
#include "Crossfader.h"

class CrossfaderComponent : public juce::Component
{
public:
    explicit CrossfaderComponent(Crossfader& processor);

    void resized() override;
    void paint(juce::Graphics&) override;

private:

    Crossfader& crossfader;

    juce::Slider crossfaderSlider;

    juce::Label deck1XfadeLabel;
    juce::Label deck2XfadeLabel;
    juce::Label crossfaderTitle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CrossfaderComponent)
};