#include "EQProcessor.h"

EQProcessor::EQProcessor()
{
}

void EQProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
	chain.prepare(spec);
	rebuildCoefficients(); // start with flat (0 dB) response
}

void EQProcessor::process(juce::dsp::AudioBlock<float>& block)
{
	// If a knob moved, update filter coefficients before processing
	if (dirty.exchange(false))
		rebuildCoefficients();

	juce::dsp::ProcessContextReplacing<float> context(block);
	chain.process(context);
}

void EQProcessor::reset()
{
	chain.reset();
}

// ─── Setters (called from UI thread) ────────────────────────────────────────

void EQProcessor::setLowGainDB(float db)
{
	lowDB.store(db);
	dirty.store(true);
}

void EQProcessor::setMidGainDB(float db)
{
	midDB.store(db);
	dirty.store(true);
}

void EQProcessor::setHighGainDB(float db)
{
	highDB.store(db);
	dirty.store(true);
}

// ─── Private: rebuild coefficients on the audio thread ──────────────────────

void EQProcessor::rebuildCoefficients()
{
    using Coeffs = juce::dsp::IIR::Coefficients<float>;

    // Load the latest dB values written by the UI thread
    float lo = lowDB.load();
    float mi = midDB.load();
    float hi = highDB.load();

    // Convert dB → linear gain (0 dB = 1.0, no change)
    float loGain = juce::Decibels::decibelsToGain(lo);
    float miGain = juce::Decibels::decibelsToGain(mi);
    float hiGain = juce::Decibels::decibelsToGain(hi);

    // LOW: shelf that boosts/cuts everything below 300 Hz
    // Q = 0.707 is the standard "Butterworth" smoothness
    *chain.get<LowShelf>().coefficients =
        *Coeffs::makeLowShelf(sampleRate, 300.0, 0.707f, loGain);

    // MID: bell/peak centred at 1 kHz
    // Q = 0.9 gives a medium-width band — good for DJ use
    *chain.get<MidPeak>().coefficients =
        *Coeffs::makePeakFilter(sampleRate, 1000.0, 0.9f, miGain);

    // HIGH: shelf that boosts/cuts everything above 4 kHz
    *chain.get<HighShelf>().coefficients =
        *Coeffs::makeHighShelf(sampleRate, 4000.0, 0.707f, hiGain);
}