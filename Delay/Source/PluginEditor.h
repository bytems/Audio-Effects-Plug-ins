/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.
    DelayAudioProcessorEditor is the object/class that represent the plug-in's UI.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Parameters.h"
#include "RotaryKnob.h"
#include "LookAndFeel.h"

//==============================================================================
/**
*/
class DelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DelayAudioProcessorEditor (DelayAudioProcessor&); //  Configure all UI elements used by Editor
    ~DelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;    // Drawing the contents of the editor
    void resized() override;                  // Position & arrange UI elements

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DelayAudioProcessor& audioProcessor;
    
    // Sub-components of Editor
    RotaryKnob gainKnob     {"Gain",     audioProcessor.apvts, gainParamID, true};
    RotaryKnob mixKnob      {"Mix",      audioProcessor.apvts, mixParamID};
    RotaryKnob delayTimeKnob{"Time",     audioProcessor.apvts, delayTimeID};
    RotaryKnob feedbackKnob {"Feedback", audioProcessor.apvts, feedbackParamID, true};
    RotaryKnob stereoKnob   {"Stereo",   audioProcessor.apvts, stereoParamID, true};
    RotaryKnob lowCutKnob   {"Low Cut",  audioProcessor.apvts, lowCutParamID};
    RotaryKnob highCutKnob  {"High Cut", audioProcessor.apvts, highCutParamID};
    
    // Grouping of sub-components
    juce::GroupComponent delayGroup, feedbackGroup, outputGroup;
    
    // Style (aka look & feel) of Editor
    MainLookAndFeel mainLF;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayAudioProcessorEditor)
};
