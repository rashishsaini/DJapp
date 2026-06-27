#pragma once

#include <JuceHeader.h>
#include "EQProcessor.h"
#include "DeckEQComponent.h"
#include "Crossfader.h"
#include "CrossfaderComponent.h"
#include "CueMixer.h"
#include "CueMixerComponent.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    // AudioAppComponent — three functions JUCE requires
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    // Your private member variables go here...
    EQProcessor deck1EQ;
    EQProcessor deck2EQ;
	Crossfader crossfader;
	CueMixer cueMixer;

   // Internal deck buffers — allocated once in prepareToPlay,
   // never reallocated during playback
    juce::AudioBuffer<float> deck1Buffer;
    juce::AudioBuffer<float> deck2Buffer;
	juce::AudioBuffer<float> masterBuffer;

    //── UI ───────────────────────────────────────────────────────

    DeckEQComponent deck1EQComp;
    DeckEQComponent deck2EQComp;

	CrossfaderComponent crossfaderComponent;
	CueMixerComponent cueMixerComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
