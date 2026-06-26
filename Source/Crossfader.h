#pragma once
#include <JuceHeader.h>

class Crossfader
{
public:
    Crossfader();

    // Call once in prepareToPlay — sets sample rate and ramp time
    void prepare(double sampleRate, int maxBlockSize);

    // Call every audio block.
    //   deck1 / deck2 — internal buffers, samples start at index 0
    //   output        — the device buffer, write from startSample
    void process(const juce::AudioBuffer<float>& deck1,
        const juce::AudioBuffer<float>& deck2,
        juce::AudioBuffer<float>& output,
        int startSample,
        int numSamples);

    void reset();

    // UI thread only.  0.0 = full deck 1,  0.5 = centre,  1.0 = full deck 2
    void  setPosition(float zeroToOne);
    float getPosition() const noexcept;

private:
    // Written from UI thread, read from audio thread
    std::atomic<float> targetPos{ 0.5f };

    // Everything below is only ever touched on the audio thread
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothed;
    float gain1{ 0.7071f };   // cos(π/4) — starts at centre position
    float gain2{ 0.7071f };   // sin(π/4)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Crossfader)
};