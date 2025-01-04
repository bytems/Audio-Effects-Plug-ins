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
const juce::ParameterID feedbackParamID("feedback", 1);
const juce::ParameterID stereoParamID("stereo", 1);
const juce::ParameterID lowCutParamID("lowCut", 1);
const juce::ParameterID highCutParamID("highCut", 1);

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
    float feedback = 0.0f;
    float panL = 0.0f;
    float panR = 1.0f;
    float lowCut = 20.0f;
    float highCut = 20000.0f;
    
    // constants
    static constexpr float minDelayTime = 5.0f;
    static constexpr float maxDelayTime = 5000.0f;   // expressed in milliseconds
    
private:
    //=====     List of Addresses where Parameters are stored in APVTS   ============
    juce::AudioParameterFloat* gainParam;
    juce::AudioParameterFloat* delayTimeParam;
    juce::AudioParameterFloat* mixParam;
    juce::AudioParameterFloat* feedbackParam;
    juce::AudioParameterFloat* stereoParam;
    juce::AudioParameterFloat* lowCutParam;
    juce::AudioParameterFloat* highCutParam;
    
    //==============================================================================
    // Mechanic to avoid discrete jumps whenever paramter is changed. solves zipper noise
    juce::LinearSmoothedValue<float> gainSmoother;
    juce::LinearSmoothedValue<float> mixSmoother;
    juce::LinearSmoothedValue<float> feedbackSmoother;
    juce::LinearSmoothedValue<float> stereoSmoother;
    juce::LinearSmoothedValue<float> lowCutSmoother;
    juce::LinearSmoothedValue<float> highCutSmoother;
    
    // Exponential Transition for Delay-Time
    float coeff = 0.0f;   // one-pole smoothing: determines how fast the smoothing happens
    float targetDelayTime = 0.0f; // The value that the one-pole filter is trying to reach
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Parameters)

};
