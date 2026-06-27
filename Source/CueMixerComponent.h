#pragma once
#include <JuceHeader.h>
#include "CueMixer.h"

class CueMixerComponent : public juce::Component
{
public:
	explicit CueMixerComponent(CueMixer& processor);
	void resized() override;
	void paint(juce::Graphics&) override;

private:
	CueMixer& cueMixer;
	juce::TextButton deck1CueBtn{ "CUE  1" };
	juce::TextButton deck2CueBtn{ "CUE  2" };
	juce::Slider     cueGainSlider;
	juce::Label      cueSectionLabel;
	juce::Label      cueGainLabel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CueMixerComponent)
};