/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
/*
 DelayAudioProcessorEditor is the object/class that represent the plug-in's UI.
 */
//==============================================================================
DelayAudioProcessorEditor::DelayAudioProcessorEditor (DelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

DelayAudioProcessorEditor::~DelayAudioProcessorEditor()
{
}

//==============================================================================
// When DAW decides to redraw the plug-in editor, it will call the paint function
// The DAW/HOST will also provide a juce::Graphics object. This object knows how to draw
// lines, teexts , etc. into the host's window.
void DelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::darkkhaki);

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (40.0f));
    g.drawFittedText ("My very First Plug-in!", getLocalBounds(), juce::Justification::centred, 1);
}

void DelayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
