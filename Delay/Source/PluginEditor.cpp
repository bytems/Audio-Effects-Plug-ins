/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
//==============================================================================
DelayAudioProcessorEditor::DelayAudioProcessorEditor (DelayAudioProcessor& p)
: AudioProcessorEditor (&p),
  audioProcessor(p),
  meter(p.levelL, p.levelR) // Grabs references to the Audio Processors "level" & passes them to meter
{
    delayGroup.setText("Delay");
    delayGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    delayGroup.addAndMakeVisible(delayTimeKnob);
    delayGroup.addChildComponent(delayNoteKnob);
    addAndMakeVisible(delayGroup);
    
    feedbackGroup.setText("Feedback");
    feedbackGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    feedbackGroup.addAndMakeVisible(feedbackKnob);
    feedbackGroup.addAndMakeVisible(stereoKnob);
    feedbackGroup.addAndMakeVisible(lowCutKnob);
    feedbackGroup.addAndMakeVisible(highCutKnob);
    addAndMakeVisible(feedbackGroup);
    
    outputGroup.setText("Output");
    outputGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    outputGroup.addAndMakeVisible(gainKnob);
    outputGroup.addAndMakeVisible(mixKnob);
    outputGroup.addAndMakeVisible(meter);
    addAndMakeVisible(outputGroup);
    
    tempoSyncButton.setButtonText("Sync");
    tempoSyncButton.setClickingTogglesState(true);
    tempoSyncButton.setBounds(0, 0, 70, 27);
    tempoSyncButton.setLookAndFeel(ButtonLookAndFeel::get());
    delayGroup.addAndMakeVisible(tempoSyncButton);
    
    // Update visibility of delay knobs based tempoSync paramter
    updateDelayKnobs(audioProcessor.params.tempoSyncParam->get());
    
    // Parameters that Editor should listen to should be registered
    audioProcessor.params.tempoSyncParam->addListener(this);
    
    
    setLookAndFeel(&mainLF);
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 330);
}

DelayAudioProcessorEditor::~DelayAudioProcessorEditor()
{
    audioProcessor.params.tempoSyncParam->removeListener(this);
    setLookAndFeel(nullptr); // Tells the editor to stop using "mainLF" as its look-and-feel
                             // so that it can be safely deallocated
}

//==============================================================================
// When DAW decides to redraw the plug-in editor, it will call the paint function
// The DAW/HOST will also provide a juce::Graphics object. This object knows how to draw
// lines, teexts , etc. into the host's window.
void DelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colors::background);

    auto rect = getLocalBounds().withHeight(40);
    g.setColour(Colors::header);
    g.fillRect(rect);
    
    // Load & decode PNG once & put into memory (slow). From now on we use image from system memory (cache)
    auto image = juce::ImageCache::getFromMemory(BinaryData::Logo_png, BinaryData::Logo_pngSize);
    
    int destWidth = image.getWidth() / 2;
    int destHeigt = image.getHeight() / 2;
    g.drawImage(image,
                getWidth()/2 - destWidth/2, 0,
                destWidth, destHeigt,
                0, 0,
                image.getWidth(), image.getHeight());
}

// This is generally where you'll want to lay out the positions of any
// subcomponents in your editor..
void DelayAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    int y = 50;
    int height = bounds.getHeight() - 60;
    
    // Position the groups
    delayGroup.setBounds(10, y, 110, height);
    outputGroup.setBounds(bounds.getWidth() - 160, y, 150, height);
    feedbackGroup.setBounds(delayGroup.getRight() + 10,
                            y,
                            outputGroup.getX() - delayGroup.getRight() - 20,
                            height);
    
    // Position the knobs inside the groups
    delayTimeKnob.setTopLeftPosition(20, 20);
    tempoSyncButton.setTopLeftPosition(20, delayTimeKnob.getBottom() + 10);
    delayNoteKnob.setTopLeftPosition(delayTimeKnob.getX(), delayTimeKnob.getY());
    feedbackKnob.setTopLeftPosition(20, 20);
    stereoKnob.setTopLeftPosition(feedbackKnob.getRight() + 20, 20);
    lowCutKnob.setTopLeftPosition(feedbackKnob.getX(), feedbackKnob.getBottom() + 10);
    highCutKnob.setTopLeftPosition(lowCutKnob.getRight() + 20, lowCutKnob.getY());
    mixKnob.setTopLeftPosition(20, 20);
    gainKnob.setTopLeftPosition(mixKnob.getX(), mixKnob.getBottom() + 10);
    meter.setBounds(outputGroup.getWidth() - 45, 30, 30, gainKnob.getBottom() - 30);
}

/**
 Warning: @function parameterValueChanged can called by different threads:
        - UI thread whey you click a button
        - Audio thread when automation is used
 UI thread, aka message thread, can directly call/modify delay knobs
 Other therads must asynchronously pass of this code to the UI thread
 */
void DelayAudioProcessorEditor::parameterValueChanged(int, float value){
    if(juce::MessageManager::getInstance()->isThisTheMessageThread())
        updateDelayKnobs(value != 0.0f);
    else{
        juce::MessageManager::callAsync([this, value]  // Lambda function FTW
        {
            updateDelayKnobs(value != 0.0f);
        });
    }
}

/**
    Hide approriate Delay Knob, and make visible other Delay knob
 */
void DelayAudioProcessorEditor::updateDelayKnobs(bool tempoSyncActive){
    delayTimeKnob.setVisible(!tempoSyncActive); // Time Knob is visible if TemoSync parameter is off
    delayNoteKnob.setVisible(tempoSyncActive);  // Note knob is ....
}
