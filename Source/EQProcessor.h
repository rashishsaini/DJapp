#pragma once
#include <JuceHeader.h>

class EQProcessor
{
public:
	EQProcessor();

	// Call before audio starts to set sample rate, block size, channels
	void prepare(const juce::dsp::ProcessSpec& spec);

	// Call every audio buffer (from the audio thread)
	void process(juce::dsp::AudioBlock<float>& block);

	// Call when the app shuts down
	void reset();

	// Call from the UI thread when a knob moves (-12 to +12 dB)
	void setLowGainDB(float db);
	void setMidGainDB(float db);
	void setHighGainDB(float db);

private:
	// Index into the three-filter chain
	enum ChainIndex { LowShelf, MidPeak, HighShelf };

	using Filter = juce::dsp::IIR::Filter<float>;
	juce::dsp::ProcessorChain<Filter, Filter, Filter> chain;

	double sampleRate = 44100.0;

	// Atomic so UI thread can write while audio thread reads
	std::atomic<float> lowDB{ 0.0f };
	std::atomic<float> midDB{ 0.0f };
	std::atomic<float> highDB{ 0.0f };
	std::atomic<bool>  dirty{ false };

	// Runs on the audio thread — safe to modify filter coefficients here
	void rebuildCoefficients();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQProcessor)
};