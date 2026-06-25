#pragma once
#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent,
                       public juce::ChangeListener,
                       private juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

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

    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;

    std::unique_ptr<juce::FileChooser> chooser;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;

    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
