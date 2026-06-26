#include "Crossfader.h"

Crossfader::Crossfader() {}

// ─────────────────────────────────────────────────────────────────────────────
void Crossfader::prepare(double sampleRate, int /*maxBlockSize*/)
{
    const float initPos = targetPos.load();

    // 50 ms linear ramp — prevents zipper noise if the fader ever jumps
    smoothed.reset(sampleRate, 0.05);
    smoothed.setCurrentAndTargetValue(initPos);

    // Pre-calculate the starting gains so the first block is correct
    const float theta = initPos * juce::MathConstants<float>::halfPi;
    gain1 = std::cos(theta);
    gain2 = std::sin(theta);
}

// ─────────────────────────────────────────────────────────────────────────────
void Crossfader::process(const juce::AudioBuffer<float>& deck1,
    const juce::AudioBuffer<float>& deck2,
    juce::AudioBuffer<float>& output,
    int startSample,
    int numSamples)
{
    // Step the smoother toward whatever position the UI wrote last.
    // We advance it sample-by-sample and keep only the final value,
    // giving constant gains for the block.  This is fine for a fader
    // moved by a human hand — changes are slower than one audio block.
    smoothed.setTargetValue(targetPos.load());

    float pos = smoothed.getCurrentValue();
    for (int i = 0; i < numSamples; ++i)
        pos = smoothed.getNextValue();

    // ── Equal power law ───────────────────────────────────────────
    //  gain1 = cos(pos × π/2)   1.0 → 0.707 → 0.0  as fader moves right
    //  gain2 = sin(pos × π/2)   0.0 → 0.707 → 1.0  as fader moves right
    //  gain1² + gain2² = 1  at every position — constant total power
    const float theta = pos * juce::MathConstants<float>::halfPi;
    gain1 = std::cos(theta);
    gain2 = std::sin(theta);

    // ── Mix into the output buffer using SIMD-optimised operations ─
    for (int ch = 0; ch < output.getNumChannels(); ++ch)
    {
        // Wrap channel index so mono deck buffers work with stereo output
        const int srcCh1 = ch % deck1.getNumChannels();
        const int srcCh2 = ch % deck2.getNumChannels();

        float* out = output.getWritePointer(ch, startSample);
        const float* src1 = deck1.getReadPointer(srcCh1);
        const float* src2 = deck2.getReadPointer(srcCh2);

        // out[i] = src1[i] * gain1
        juce::FloatVectorOperations::copyWithMultiply(out, src1, gain1, numSamples);

        // out[i] += src2[i] * gain2
        juce::FloatVectorOperations::addWithMultiply(out, src2, gain2, numSamples);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void Crossfader::reset()
{
    // Snap smoother to current target — avoids a stale ramp on next prepare
    smoothed.setCurrentAndTargetValue(targetPos.load());
}

void Crossfader::setPosition(float p)
{
    targetPos.store(juce::jlimit(0.0f, 1.0f, p));
}

float Crossfader::getPosition() const noexcept
{
    return targetPos.load();
}