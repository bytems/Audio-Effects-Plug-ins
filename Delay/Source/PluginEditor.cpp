/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
//==============================================================================
DelayAudioProcessorEditor::DelayAudioProcessorEditor (DelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    delayGroup.setText("Delay");
    delayGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    delayGroup.addAndMakeVisible(delayTimeKnob);
    addAndMakeVisible(delayGroup);
    
    feedbackGroup.setText("Feedback");
    feedbackGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    addAndMakeVisible(feedbackGroup);
    
    outputGroup.setText("Output");
    outputGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    outputGroup.addAndMakeVisible(gainKnob);
    outputGroup.addAndMakeVisible(mixKnob);
    addAndMakeVisible(outputGroup);
    
    //addAndMakeVisible(gainKnob);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 300);
    setLookAndFeel(&mainLF);
}

DelayAudioProcessorEditor::~DelayAudioProcessorEditor()
{
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
    delayTimeKnob.setTopLeftPosition(20, 10);
    mixKnob.setTopLeftPosition(20, 10);
    gainKnob.setTopLeftPosition(mixKnob.getX(), mixKnob.getBottom() + 10);
}
