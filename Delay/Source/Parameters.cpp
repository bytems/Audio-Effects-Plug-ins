/*
  ==============================================================================

    Parameters.cpp
    
    Defines all the parameters used by the Delay Audio Proc plug-in
  ==============================================================================
*/

#include "Parameters.h"

const juce::ParameterID gainParamID{"gain",1};

//==============================================================================
template<typename T>
static void castParameter(juce::AudioProcessorValueTreeState& aptvs,
                          const juce::ParameterID& id, T& destination)
{
    // Looking parameter from APTVS takes time, may involve doing multiple string comparisons
    // Let's Cache (memorize) location (pointer) of parameter, to make look-up fast.
    destination = dynamic_cast<T>(aptvs.getParameter(id.getParamID()));
    jassert(destination);
}

//==============================================================================
// Constructor
Parameters::Parameters(juce::AudioProcessorValueTreeState& apvts)
{
    // Looking parameter from APTVS takes time, may involve doing multiple string comparisons
    // Let's Cache (memorize) location (pointer) of parameter, to make look-up fast.
    castParameter(apvts, gainParamID, gainParam);
}

//==============================================================================
// This function creates a ParameterLayout object that describes the plug-in paramters that should be added to the APTVS
juce::AudioProcessorValueTreeState::ParameterLayout Parameters::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // AudioParamterFloat is the object that describes the actual plug-in parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
                                                           gainParamID,
                                                           "Output Gain rip",
                                                           juce::NormalisableRange<float> {-12.0f, 12.0f},
                                                           0.00f));
    return layout;
}

//==============================================================================
void Parameters::prepareToPlay(double sampleRate) noexcept
{
    // gainSmoother needs to know how long it should take to transition from previous parameter value to new one
    double duration = 0.02;  // in seconds
    // If smoothing time too short shor, you'll hear zipper noise
    // if smoothing time too long, you';; hear sound fade in or out
    // 20 ms at 48KHz is 960 samples (a good compromise)
    gainSmoother.reset(sampleRate, duration);
}


void Parameters::reset() noexcept
{
    gain = 0.0f;
    gainSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain((gainParam->get())));
}
// This function updates the parameters from the latest APTVS source - usally called once per block
void Parameters::update() noexcept
{
    /*
    float gainInDecibels = gainParam->get(); // reads the current value from the parameter (address in APTVS)
    float newGain = juce::Decibels::decibelsToGain(gainInDecibels);
    gainSmoother.setTargetValue(newGain);// Tells smoother about new value. If it differs, smoother will get to work
     */
    gainSmoother.setTargetValue(juce::Decibels::decibelsToGain(gainParam->get()));
}

// Called once per sample
void Parameters::smoothen() noexcept
{
    gain = gainSmoother.getNextValue();
}
