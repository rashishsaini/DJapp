#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : deck1EQComp("DECK 1", juce::Colour(0xff00aaff)),  // blue
      deck2EQComp("DECK 2", juce::Colour(0xffff5500)),   // orange
      crossfaderComponent(crossfader),
	  cueMixerComponent(cueMixer)
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

    addAndMakeVisible(cueMixerComponent);

    setSize(720, 720);

    // Tell JUCE we want stereo in + stereo out from the audio device
    setAudioChannels(2, 4);
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


    deck1Buffer.setSize(2, samplesPerBlockExpected);
    deck2Buffer.setSize(2, samplesPerBlockExpected);
    masterBuffer.setSize(2, samplesPerBlockExpected);

    deck1Buffer.clear();
    deck2Buffer.clear();
    masterBuffer.clear();

    crossfader.prepare(spec.sampleRate, spec.maximumBlockSize);
    cueMixer.prepare(spec.sampleRate, spec.maximumBlockSize);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Audio loop — called thousands of times per second
// ─────────────────────────────────────────────────────────────────────────────
void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& info)
{

    auto* buffer = info.buffer;
    const int start = info.startSample;
    const int numSamps = info.numSamples;
    const int numCh = buffer->getNumChannels();

    // ── Step 1: fill per-deck buffers ────────────────────────────
    // Deck 1: copying empty output as a placeholder (replace with AudioTransportSource)
    for (int ch = 0; ch < juce::jmin(numCh, deck1Buffer.getNumChannels()); ++ch)
        deck1Buffer.copyFrom(ch, 0, *info.buffer, ch, start, numSamps);

    // Deck 2: silence placeholder (replace with AudioTransportSource)
    deck2Buffer.clear();

    // ── Step 2: EQ each deck ────────────────────────────────────
    {
        juce::dsp::AudioBlock<float> b(deck1Buffer);
        auto sub = b.getSubBlock(0, static_cast<size_t>(numSamps));
        deck1EQ.process(sub);
    }
    {
        juce::dsp::AudioBlock<float> b(deck2Buffer);
        auto sub = b.getSubBlock(0, static_cast<size_t>(numSamps));
        deck2EQ.process(sub);
    }

    // ── Step 3: tap the PFL cue bus (PRE-crossfader) ────────────
    cueMixer.process(deck1Buffer, deck2Buffer, numSamps);

    info.buffer->copyFrom(
        2,
        start,
        cueMixer.getCueBuffer(),
        0,
        0,
        numSamps);

    info.buffer->copyFrom(
        3,
        start,
        cueMixer.getCueBuffer(),
        1,
        0,
        numSamps);

    // ── Step 4: crossfade into the master output ─────────────────
    crossfader.process(deck1Buffer, deck2Buffer, masterBuffer, start, numSamps);

    info.buffer->copyFrom(
        0,
        start,
        masterBuffer,
        0,
        0,
        numSamps);

    info.buffer->copyFrom(
        1,
        start,
        masterBuffer,
        1,
        0,
        numSamps);
}

void MainComponent::releaseResources()
{
    deck1EQ.reset();
    deck2EQ.reset();

	crossfader.reset();
	cueMixer.reset();
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
	auto cueArea = area.removeFromTop(80); // space for cue mixer

	auto decksArea = area;   // remaining space for the two decks   

    const int halfW = decksArea.getWidth() / 2;
    deck1EQComp.setBounds(decksArea.removeFromLeft(halfW).reduced(5));
    deck2EQComp.setBounds(decksArea.reduced(5));

    crossfaderComponent.setBounds(crossfaderArea.reduced(40,5));
    cueMixerComponent.setBounds(cueArea.reduced(0, 4));
}
