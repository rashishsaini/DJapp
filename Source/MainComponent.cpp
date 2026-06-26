#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() : openButton("Open File"), 
                                 state(Stopped), 
                                 thumbnailCache(5), 
                                 thumbnail(512, formatManager, thumbnailCache)
{
    
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

    addAndMakeVisible(&volumeSlider);
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.5); // Set initial volume to 50%
    volumeSlider.onValueChange = [this] {updateGain(); }; // Update gain when the slider value changes
    addAndMakeVisible(&volumeLabel);
    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.attachToComponent(&volumeSlider, true); // Attach the label to the slider

    addAndMakeVisible(&gainSlider);
    gainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    gainSlider.setRange(0.0, 2.0, 0.01); // Gain range from 0.0 to 2.0
    gainSlider.setValue(1.0); // Set initial gain to 1.0 (no change)
    gainSlider.onValueChange = [this] {updateGain(); }; // Update gain when
    addAndMakeVisible(&gainLabel);
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.attachToComponent(&gainSlider, true); // Attach the label to the slider

    setSize (800, 600);
    
    formatManager.registerBasicFormats(); // Register basic audio formats (WAV, AIFF, MP3, etc.)
    transportSource.addChangeListener(this); // Add change listener to the transport source
    thumbnail.addChangeListener(this); // Add change listener to the thumbnail
    
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
    startTimer(40);
 }

MainComponent::~MainComponent()
{
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
                thumbnail.setSource(new juce::FileInputSource(file));
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

void MainComponent::resized()
{
    openButton.setBounds (10, 10, getWidth() - 20, 40); // Position the button at the top of the component
    playButton.setBounds (10, 60, getWidth() - 20, 40); // Position the play button below the open button
    stopButton.setBounds (10, 110, getWidth() - 20, 40); // Position the stop button below the play button

    volumeSlider.setBounds(80, 330, getWidth() - 90, 20); 
    gainSlider.setBounds(80, 360, getWidth() - 90, 20);
}

void MainComponent::changeListenerCallback (juce::ChangeBroadcaster* source)
// {
//     if (source == &transportSource)
//     {
//         if (transportSource.isPlaying())
//             changeState (Playing);
//         else if ((state == Stopping) || (state == Playing))
//             changeState (Stopped);
//         else if (Pausing == state)
//             changeState (Paused);
//     }
// }
{
    if (source == &transportSource)
    {
        transportSourceChanged();
    }
    else if (source == &thumbnail)
    {
        thumbnailChanged();
    }
}

void MainComponent::transportSourceChanged()
{
    if (transportSource.isPlaying())
        changeState(Playing);
    else if ((state == Stopping) || (state == Playing))
        changeState(Stopped);
    else if (Pausing == state)
        changeState(Paused);
}

void MainComponent::thumbnailChanged()
{
    // This function will be called when the thumbnail has changed.
    // You can use this to trigger a repaint or update the UI as needed.
    repaint(); // For example, you might want to repaint the component to show the updated thumbnail.
}

void MainComponent::paint (juce::Graphics& g)
{
    juce::Rectangle<int> thumbnailBounds (10, 160, getWidth() - 20, 150 );
    if (thumbnail.getNumChannels()== 0)
        paintIfNoFileLoaded (g, thumbnailBounds);
    else
        paintIfFileLoaded (g, thumbnailBounds);
}
void MainComponent::paintIfNoFileLoaded (juce::Graphics& g, juce::Rectangle<int>& thumbnailBounds)
{
    g.setColour (juce::Colours::darkgrey);
    g.fillRect (thumbnailBounds);
    g.setColour (juce::Colours::white);
    g.drawFittedText ("No File Loaded", thumbnailBounds, juce::Justification::centred, 1);
}
void MainComponent::paintIfFileLoaded (juce::Graphics& g, juce::Rectangle<int>& thumbnailBounds)
{
    g.setColour (juce::Colours::white);
    g.fillRect (thumbnailBounds);
    g.setColour (juce::Colours::blue);
    thumbnail.drawChannels
    (
        g, 
        thumbnailBounds, 
        0.0, 
        thumbnail.getTotalLength(), 
        0.5f //vertical zoom factor
    );
    auto audioLength = transportSource.getLengthInSeconds();
    
    if (audioLength > 0.0) // Protect against dividing by zero if no file is loaded
    {
        // Find the ratio of how far along the song is (between 0.0 and 1.0)
        auto audioPosition = transportSource.getCurrentPosition();
        auto playheadRatio = audioPosition / audioLength;
        
        // Convert that ratio into a pixel X coordinate
        auto playheadX = thumbnailBounds.getX() + (playheadRatio * thumbnailBounds.getWidth());
        
        // Draw the line (e.g., a green vertical line)
        g.setColour (juce::Colours::green);
        g.drawVerticalLine (static_cast<int>(playheadX), thumbnailBounds.getY(), thumbnailBounds.getBottom());
    }
}

void MainComponent::timerCallback() 
{
    if (state == Playing)
    {
        // You can update the UI here, for example, to show the current playback position.
        // For now, we'll just repaint the component to reflect any changes.
        repaint();
    }
}


void MainComponent::updateGain()
{
    float masterVolume = volumeSlider.getValue(); // Get the value from the volume slider
    float inputGain = gainSlider.getValue(); // Get the value from the gain slider
    transportSource.setGain(masterVolume * inputGain); // Set the transport source gain based on both sliders
}
