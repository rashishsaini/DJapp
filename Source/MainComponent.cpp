#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : deck1EQComp("DECK 1", juce::Colour(0xff00aaff)),  // blue
      deck2EQComp("DECK 2", juce::Colour(0xffff5500)),   // orange
      crossfaderComponent(crossfader)
{
    // ── Wire deck 1 knobs → DSP ──────────────────────────────────
    deck1EQComp.onLowChanged = [this](float db) { deck1EQ.setLowGainDB(db);  };
    deck1EQComp.onMidChanged = [this](float db) { deck1EQ.setMidGainDB(db);  };
    deck1EQComp.onHighChanged = [this](float db) { deck1EQ.setHighGainDB(db); };

    // ── Wire deck 2 knobs → DSP ──────────────────────────────────
    deck2EQComp.onLowChanged = [this](float db) { deck2EQ.setLowGainDB(db);  };
    deck2EQComp.onMidChanged = [this](float db) { deck2EQ.setMidGainDB(db);  };
    deck2EQComp.onHighChanged = [this](float db) { deck2EQ.setHighGainDB(db); };

    addAndMakeVisible(deck1EQComp);
    addAndMakeVisible(deck2EQComp);

    addAndMakeVisible(crossfaderComponent);

    setSize(720, 600);

    // Tell JUCE we want stereo in + stereo out from the audio device
    setAudioChannels(2, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();   // MUST be called before anything else is destroyed
}

// ─────────────────────────────────────────────────────────────────────────────
//  Audio setup — called once when audio starts
// ─────────────────────────────────────────────────────────────────────────────
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlockExpected);
    spec.numChannels = 2;

    deck1EQ.prepare(spec);
    deck2EQ.prepare(spec);

    crossfader.prepare(spec.sampleRate, spec.maximumBlockSize);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Audio loop — called thousands of times per second
// ─────────────────────────────────────────────────────────────────────────────
void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& info)
{
    // ── In a real DJ app: fill info.buffer from each deck's AudioTransportSource
    // ── For now, this passes live microphone/line-in audio through deck 1's EQ
    // ── so you can immediately hear the knobs working.

    auto* buffer = info.buffer;
    auto  start = static_cast<size_t>(info.startSample);
    auto  numSamps = static_cast<size_t>(info.numSamples);

    juce::dsp::AudioBlock<float> block(*buffer);
    auto sub = block.getSubBlock(start, numSamps);

    deck1EQ.process(sub);   // apply deck 1 EQ to the audio

    // deck2EQ.process(...) would process deck 2's separate audio buffer
}

void MainComponent::releaseResources()
{
    deck1EQ.reset();
    deck2EQ.reset();

	crossfader.reset();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff080818));   // very dark navy

    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(juce::Font(18.0f, juce::Font::bold));
    g.drawText("DJ EQ", getLocalBounds().removeFromTop(38),
                juce::Justification::centred, true);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(38);              // space for title
    auto crossfaderArea = area.removeFromBottom(120); // space for crossfader

	auto decksArea = area;   // remaining space for the two decks   

    const int halfW = decksArea.getWidth() / 2;
    deck1EQComp.setBounds(decksArea.removeFromLeft(halfW).reduced(5));
    deck2EQComp.setBounds(decksArea.reduced(5));

    crossfaderComponent.setBounds(crossfaderArea.reduced(40,5));
}
