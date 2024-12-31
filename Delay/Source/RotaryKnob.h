/*
  ==============================================================================

    RotaryKnob.h
   
    UI Component class that provides a combined pair of:
        - JUCE slider
        - JUCE label

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class RotaryKnob  : public juce::Component
{
public:
    RotaryKnob(const juce::String& text,
               juce::AudioProcessorValueTreeState& apvts,
               const juce::ParameterID& parameterID);
    ~RotaryKnob() override;

    void resized() override;
    
    juce::Slider slider;
    juce::Label label;
    
    // Object/Variable to attach UI component & Plug-in Parameter
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RotaryKnob)
};
