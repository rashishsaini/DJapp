#pragma once
#include <JuceHeader.h>

/**
 *  CueMixer — Pre-Fader Listen (PFL) headphone bus.
 *
 *  Receives post-EQ, pre-crossfader audio from both decks and mixes
 *  only the cued deck(s) into an internal stereo buffer.  The master
 *  output path is never touched.
 *
 *  Threading model (mirrors EQProcessor / Crossfader):
 *    UI thread   → writes std::atomics via setDeck*Cue / setCueGain
 *    Audio thread → reads atomics and writes cueBuffer inside process()
 */
class CueMixer
{
public:
    CueMixer();

    // ── Lifecycle ────────────────────────────────────────────────
    /** Call once in prepareToPlay. Pre-allocates the internal cue buffer. */
    void prepare(double sampleRate, int maxBlockSize);

    /** Call in releaseResources. Snaps the gain smoother and clears the buffer. */
    void reset();

    // ── Per-block DSP (audio thread only) ─────────────────────────
    /**
    Fills the internal cue buffer from whichever decks are enabled.
    */
    void process(const juce::AudioBuffer<float>& deck1,
        const juce::AudioBuffer<float>& deck2,
        int numSamples);

    /**
     *  Returns the stereo cue buffer written by the last process() call.
     *  Valid for numSamples samples starting at index 0.
     *  Only call this from the audio thread, and only after process().
     *
     *  When multi-output hardware is added, copy this buffer into the
     *  headphone output channel pair in getNextAudioBlock().
     */
    const juce::AudioBuffer<float>& getCueBuffer() const noexcept;

    // ── Parameters — written from UI thread, read on audio thread ─
    void setDeck1Cue(bool enabled);     ///< Route Deck 1 into the cue bus
    void setDeck2Cue(bool enabled);     ///< Route Deck 2 into the cue bus

    /**
     *  Headphone volume. Linear gain: 0.0 = mute, 1.0 = unity, 2.0 ≈ +6 dB.
     *  Changes are smoothed over 30 ms to prevent clicks.
     */
    void setCueGain(float linearGain);

    bool  getDeck1Cue() const noexcept;
    bool  getDeck2Cue() const noexcept;
    float getCueGain()  const noexcept;

private:
    // Written by UI thread, read by audio thread
    std::atomic<bool>  deck1CueOn{ false };
    std::atomic<bool>  deck2CueOn{ false };
    std::atomic<float> targetGain{ 1.0f };

    // Only ever touched on the audio thread
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedGain;

    // Pre-allocated stereo result buffer — never resized inside process()
    juce::AudioBuffer<float> cueBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CueMixer)
};