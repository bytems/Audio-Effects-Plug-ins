/*
  ==============================================================================

   The Parameter Class
 
   This class defines all the parameters that will be used by the plug-in.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h> // so C++ compiler knows what juce:: means

// Define the paramater ID as a constant that you can refer to later
const juce::ParameterID gainParamID{"gain",1};
const juce::ParameterID delayTimeID("delayTime", 1);
const juce::ParameterID mixParamID("mix", 1);

class Parameters
{
public:
    //==============================================================================
    /*
        Parameters need to be placed in a "Layout", when creating APVTS
     */
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    //==============================================================================
    Parameters(juce::AudioProcessorValueTreeState& apvts);
    
    
    //==============================================================================
    void prepareToPlay(double sampleRate) noexcept;
    void update() noexcept;
    void reset() noexcept;
    void smoothen() noexcept;
    
    // The public-facing variables to be used in Processing block
    float gain = 0.0f;
    float delayTime = 0.0f;
    float mix = 1.0f;  // % of wet mixed into dry
    
    // constants
    static constexpr float minDelayTime = 0.1f;
    static constexpr float maxDelayTime = 5000.0f;   // expressed in milliseconds
    
private:
    //=====     List of Addresses where Parameters are stored in APVTS   ============
    juce::AudioParameterFloat* gainParam;
    juce::AudioParameterFloat* delayTimeParam;
    juce::AudioParameterFloat* mixParam;
    
    //==============================================================================
    // Mechanic to avoid discrete jumps whenever paramter is changed. solves zipper noise
    juce::LinearSmoothedValue<float> gainSmoother;
    juce::LinearSmoothedValue<float> mixSmoother;
    
    // Exponential Transition for Delay-Time
    float coeff = 0.0f;   // one-pole smoothing: determines how fast the smoothing happens
    float targetDelayTime = 0.0f; // The value that the one-pole filter is trying to reach

};
