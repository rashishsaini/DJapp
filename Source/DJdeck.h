#pragma once
#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class DJdeck  : public juce::Component,
                public juce::ChangeListener,
                private juce::Timer
{
public:
    //==============================================================================
    DJdeck(juce::AudioFormatManager& formatManagerToUse);
    ~DJdeck() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) ;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) ;
    void releaseResources() ;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

private:
    // define an enum to represent the transport state
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Pausing,
        Paused,
        Stopping
    };

    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();

    void changeState(TransportState newState);

    void transportSourceChanged();
    void thumbnailChanged();
    void paintIfNoFileLoaded (juce::Graphics& g, juce::Rectangle<int>& thumbnailBounds);
    void paintIfFileLoaded (juce::Graphics& g, juce::Rectangle<int>& thumbnailBounds);
    void timerCallback() override;

    void updateGain();

    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;

    std::unique_ptr<juce::FileChooser> chooser;

    juce::AudioFormatManager& formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;

    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    juce::Slider gainSlider;
    juce::Label gainLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DJdeck)
};
