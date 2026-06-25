#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() : openButton("Open File"), state(Stopped)
{
    // Make sure you set the size of the component after
    // you add any child components.
    // Add the open button to the component
    addAndMakeVisible (&openButton);
    openButton.setButtonText("Open File");
    openButton.onClick = [this] { openButtonClicked(); }; // Set the button click callback
    
    addAndMakeVisible(&playButton);
    playButton.setButtonText("Play");
    playButton.onClick = [this] { playButtonClicked(); }; // Set the button click callback
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green); // Set the button color to green
    playButton.setEnabled(false); // Disable the play button initially

    addAndMakeVisible(&stopButton);
    stopButton.setButtonText("Stop");   
    stopButton.onClick = [this] { stopButtonClicked(); }; // Set the button click callback
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red); // Set the button color to red
    stopButton.setEnabled(false); // Disable the stop button initially

    setSize (800, 600);

    formatManager.registerBasicFormats(); // Register basic audio formats (WAV, AIFF, MP3, etc.)
    transportSource.addChangeListener(this); // Add change listener to the transport source

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
    && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (0, 2);
    }
 }

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}

void MainComponent::changeState(TransportState newState)
    {
        if (state != newState)
        {
            state = newState;

            switch (state)
            {
                case Stopped:                        
                    playButton.setButtonText("Play");
                    stopButton.setButtonText("Stop");
                    stopButton.setEnabled(false);
                    transportSource.setPosition (0.0);
                    break;

                case Starting:                          
                    //playButton.setEnabled (false);
                    transportSource.start();
                    break;

                case Playing: 
                    playButton.setButtonText("Pause");
                    stopButton.setButtonText("Stop");
                    stopButton.setEnabled (true);
                    break;

                case Pausing:                          
                    transportSource.stop();
                    break;

                case Paused:
                    playButton.setButtonText("Resume");
                    stopButton.setButtonText("Return to Start");
                    stopButton.setEnabled (true);
                    break;

                case Stopping:                         
                    transportSource.stop();
                    break;
            }
        }
    }


void MainComponent::openButtonClicked()
{
    chooser=std::make_unique<juce::FileChooser>("Select a wave file to play...",
                                                 juce::File{},
                                                 "*.wav");
    auto chooserFlags = juce::FileBrowserComponent::openMode
                      | juce::FileBrowserComponent::canSelectFiles;
    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file != juce::File{})
        {
            auto* reader = formatManager.createReaderFor(file);
            if (reader != nullptr)
            {
                auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
                transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
                playButton.setEnabled(true);
                readerSource.reset(newSource.release());
            }

        }
    });

}

void MainComponent::playButtonClicked()
{
    if ((state == Stopped) || (state == Paused))
        changeState (Starting);
    else if (state == Playing)
        changeState (Pausing);
}

void MainComponent::stopButtonClicked()
{
    if (state == Paused)
        changeState (Stopped);
    else 
        changeState (Stopping);
}



void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    transportSource.getNextAudioBlock (bufferToFill);
}


//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    openButton.setBounds (10, 10, getWidth() - 20, 40); // Position the button at the top of the component
    playButton.setBounds (10, 60, getWidth() - 20, 40); // Position the play button below the open button
    stopButton.setBounds (10, 110, getWidth() - 20, 40); // Position the stop button below the play button
}

void MainComponent::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {
        if (transportSource.isPlaying())
            changeState (Playing);
        else if ((state == Stopping) || (state == Playing))
            changeState (Stopped);
        else if (Pausing == state)
            changeState (Paused);
    }
}

