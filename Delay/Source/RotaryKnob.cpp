/*
  ==============================================================================

    RotaryKnob.cpp
    Created: 29 Dec 2024 10:49:24pm
    Author:  mohamed saleh

  ==============================================================================
*/

#include <JuceHeader.h>
#include "RotaryKnob.h"

//==============================================================================
// In your constructor, you should add any child components, and
// initialise any special settings that your component needs.
RotaryKnob::RotaryKnob(const juce::String& text,
                       juce::AudioProcessorValueTreeState& apvts,
                       const juce::ParameterID& parameterID)
    : attachment(apvts, parameterID.getParamID(), slider) 
{
    
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 16);
    slider.setBounds(0, 0, 70, 86);
    addAndMakeVisible(slider); // makes the slide a "child component" of the editor
    
    label.setText(text, juce::NotificationType::dontSendNotification);
    label.setJustificationType(juce::Justification::horizontallyCentred);
    label.setBorderSize(juce::BorderSize<int>{0,0,2,0});
    label.attachToComponent(&slider, false);
    addAndMakeVisible(label);
    
    setSize(70, 110);  // Label is 24 pixels high. So total height is 86 + 24 = 110

}

RotaryKnob::~RotaryKnob()
{
}


void RotaryKnob::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    slider.setTopLeftPosition(0, 24);  // Need to make space for the label (24 pixels attached to top of slider
}
