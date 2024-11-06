/*
  ==============================================================================

   The Parameter Class
 
   This class defines all the parameters that will be used by the plug-in.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h> // so C++ compiler knows what juce:: means

class Parameters
{
public:
    Parameters(juce::AudioProcessorValueTreeState& apvts);
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // noexcept means we're guaranteeing this funtion wont throw an exception.
    // Compiler can generate more optimal code.
    // Good Habit: Mark all audio processing functions 'noexcept' to get that extra speed boost
    void prepareToPlay(double sampleRate) noexcept;
    void update() noexcept;
    void reset() noexcept;
    void smoothen() noexcept;
    
    float gain = 0.0f;
    
    // constants
    static constexpr float minDelayTime = 5.0f;
    static constexpr float maxDelayTime = 5000.0f;
private:
    juce::LinearSmoothedValue<float> gainSmoother;
    // List of Addresses where Parameters are stored in APVTS
    juce::AudioParameterFloat* gainParam;
};
