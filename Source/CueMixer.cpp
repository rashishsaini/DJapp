#include "CueMixer.h"

CueMixer::CueMixer() {}

// ─────────────────────────────────────────────────────────────────────────────
void CueMixer::prepare(double sampleRate, int maxBlockSize)
{
    // 30 ms linear ramp — prevents clicks when the headphone knob is turned
    smoothedGain.reset(sampleRate, 0.03);
    smoothedGain.setCurrentAndTargetValue(targetGain.load());

    // Allocate here once — process() must never allocate
    cueBuffer.setSize(2, maxBlockSize);
    cueBuffer.clear();
}

// ─────────────────────────────────────────────────────────────────────────────
void CueMixer::reset()
{
    smoothedGain.setCurrentAndTargetValue(targetGain.load());
    cueBuffer.clear();
}

// ─────────────────────────────────────────────────────────────────────────────
void CueMixer::process(const juce::AudioBuffer<float>& deck1,
    const juce::AudioBuffer<float>& deck2,
    int numSamples)
{
    // ── 1. Advance the gain smoother ──────────────────────────────
    // Constant gain per block is fine for headphone monitoring.
    smoothedGain.setTargetValue(targetGain.load());
    float gain = smoothedGain.getCurrentValue();
    for (int i = 0; i < numSamples; ++i)
        gain = smoothedGain.getNextValue();

    // ── 2. Start from silence ────────────────────────────────────
    // Only clear the live samples; the rest of the pre-allocated buffer
    // is never read by the caller.
    cueBuffer.clear(0, numSamples);

    // ── 3. Accumulate whichever decks are cued ───────────────────
    // Snapshot atomic flags once so they're consistent for the whole block.
    const bool d1 = deck1CueOn.load();
    const bool d2 = deck2CueOn.load();

    for (int ch = 0; ch < cueBuffer.getNumChannels(); ++ch)
    {
        float* out = cueBuffer.getWritePointer(ch);

        if (d1)
        {
            const float* src = deck1.getReadPointer(ch % deck1.getNumChannels());
            // addWithMultiply: out[i] += src[i] * gain  (SIMD-optimised)
            juce::FloatVectorOperations::addWithMultiply(out, src, gain, numSamples);
        }

        if (d2)
        {
            const float* src = deck2.getReadPointer(ch % deck2.getNumChannels());
            juce::FloatVectorOperations::addWithMultiply(out, src, gain, numSamples);
        }
    }
    // Note: when both decks are cued the signals sum, matching real DJ-mixer
    // behaviour (CUE buttons are additive).  The headphone gain knob lets
    // the DJ manage the combined level.
}

// ─────────────────────────────────────────────────────────────────────────────
const juce::AudioBuffer<float>& CueMixer::getCueBuffer() const noexcept
{
    return cueBuffer;
}

// ─────────────────────────────────────────────────────────────────────────────
void CueMixer::setDeck1Cue(bool enabled) { deck1CueOn.store(enabled); }
void CueMixer::setDeck2Cue(bool enabled) { deck2CueOn.store(enabled); }

void CueMixer::setCueGain(float g)
{
    // Cap at 2.0 (≈ +6 dB); the lower bound of 0.0 gives a clean mute
    targetGain.store(juce::jlimit(0.0f, 2.0f, g));
}

bool  CueMixer::getDeck1Cue() const noexcept { return deck1CueOn.load(); }
bool  CueMixer::getDeck2Cue() const noexcept { return deck2CueOn.load(); }
float CueMixer::getCueGain()  const noexcept { return targetGain.load(); }