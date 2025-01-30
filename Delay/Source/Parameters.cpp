/*
  ==============================================================================

    Parameters.cpp
    
    Defines all the parameters used by the Delay Audio Proc plug-in
  ==============================================================================
*/

#include "Parameters.h"
#include "DSP.h"

//===================== Static Helper Functions =================================
template<typename T>
static void castParameter(juce::AudioProcessorValueTreeState& aptvs,
                          const juce::ParameterID& id, T& destination)
{
    // Looking parameter from APTVS takes time, may involve doing multiple string comparisons
    // Let's Cache (memorize) location (pointer) of parameter, to make look-up fast.
    destination = dynamic_cast<T>(aptvs.getParameter(id.getParamID()));
    jassert(destination);
}


// string-from-value functions
static juce::String stringFromMilliseconds(float value, int){
    if(value < 10.0f)        return juce::String(value, 2) + " ms";
    else if(value < 100.0f)  return juce::String(value, 1) + " ms";
    else if(value < 1000.0f) return juce::String(int(value)) + " ms";
    else                     return juce::String(value * 0.001f, 2) + " s";
}

static float millisecondsFromString(const juce::String& text)
{
    float value = text.getFloatValue();
    if(!text.endsWithIgnoreCase("ms")){
        if(text.endsWithIgnoreCase("s") || value < Parameters::minDelayTime)
            return value * 1000.0f;
    }
    return value;
}

// Will be used to display the cutoff frequency in Hz, or if larger than 1000 Hz, in kHz
static juce::String stringFromHz(float value, int){
    if(value < 1000.0f)    return juce::String(int(value)) + " Hz";
    else if(value < 10000) return juce::String(value /1000.0f,2) + " k";
    else                   return juce::String(value /1000.0f,1) + " k";
}

static float hzFromString(const juce::String& str){
    float value = str.getFloatValue();
    if(value < 20.0f)
        return value * 1000.0f;
    return value;
}

static juce::String stringFromDecibels(float value, int){
    return juce::String(value, 1) + " dB";
}

static juce::String stringFromPercent(float value, int){
    return juce::String(int(value)) + " %";
}

//==============================================================================
/* Constructor
 * Caches locations of all paramters from APVTS
 */
Parameters::Parameters(juce::AudioProcessorValueTreeState& apvts)
{
    // Looking parameter from APTVS takes time, may involve doing multiple string comparisons
    // Let's Cache (memorize) location (pointer) of parameter, to make look-up fast.
    castParameter(apvts, gainParamID, gainParam);
    castParameter(apvts, delayTimeID, delayTimeParam);
    castParameter(apvts, mixParamID, mixParam);
    castParameter(apvts, feedbackParamID, feedbackParam);
    castParameter(apvts, stereoParamID, stereoParam);
    castParameter(apvts, lowCutParamID, lowCutParam);
    castParameter(apvts, highCutParamID, highCutParam);
    castParameter(apvts, tempoSyncParamID, tempoSyncParam);
    castParameter(apvts, delayNoteParamID, delayNoteParam);
    castParameter(apvts, bypassParamID, bypassParam);
}

//==============================================================================
// This function creates a ParameterLayout object that describes the plug-in paramters that should be added to the APTVS
juce::AudioProcessorValueTreeState::ParameterLayout Parameters::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // AudioParamterFloat is the object that describes the actual plug-in parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
                    gainParamID,   //
                    "Output Gain",
                    juce::NormalisableRange<float> {-12.0f, 12.0f},
                    0.00f,
                    juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromDecibels)
                    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
                    mixParamID,
                    "Mix",
                    juce::NormalisableRange<float> {0.0f, 100.0f, 1.0f},
                    100.0f,
                    juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercent)
                    // Tells JUCE to use the string-from-value function when host asks for a textual representation of the parameters value
                    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
                    delayTimeID,
                    "DelayTime",
                    juce::NormalisableRange<float> {minDelayTime, maxDelayTime, 0.001f, 0.25f},
                    100.0f,
                    juce::AudioParameterFloatAttributes()
                        .withStringFromValueFunction(stringFromMilliseconds)
                        .withValueFromStringFunction(millisecondsFromString)
                    // Tells JUCE to use the msfrStr function when host updates the parameters value
                    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
                    feedbackParamID,
                    "Feedback",
                    juce::NormalisableRange<float> {-100.0f, 100.0f, 1.0f},
                    0.0f,
                    juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercent)
                    // Tells JUCE to use the string-from-value function when host asks for a textual representation of the parameters value
                    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
                    stereoParamID,
                    "Stereo",
                    juce::NormalisableRange<float> {-100.0f, 100.0f, 1.0f},
                    0.0f,
                    juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercent)
                    // Tells JUCE to use the string-from-value function when host asks for a textual representation of the parameters value
                    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
                    lowCutParamID,
                    "Low Cut",
                    juce::NormalisableRange<float> {20.0f, 20000.0f, 1.0f, 0.3f},
                    20.0f,
                    juce::AudioParameterFloatAttributes()
                           .withStringFromValueFunction(stringFromHz)
                           .withValueFromStringFunction(hzFromString)
                    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
                    highCutParamID,
                    "High Cut",
                    juce::NormalisableRange<float> {20.0f, 20000.0f, 1.0f, 0.3f},
                    20000.0f,
                    juce::AudioParameterFloatAttributes()
                           .withStringFromValueFunction(stringFromHz)
                           .withValueFromStringFunction(hzFromString)
                    ));
    layout.add(std::make_unique<juce::AudioParameterBool>(tempoSyncParamID,"Tympo Sync",false));
    layout.add(std::make_unique<juce::AudioParameterBool>(bypassParamID, "Bypass", false));
    
    juce::StringArray noteLengths{"1/32","1/16 trip", "1/32 dot", "1/16", "1/8 trip", "1/16 dot",
                                  "1/8", "1/4 trip",  "1/8 dot",  "1/4",  "1/2 trip", "1/4 dot",
                                  "1/2", "1/1 trip",  "1/2 dot",  "1/1"};
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(delayNoteParamID, "Delay Note", noteLengths, 9));
    
    return layout;
}

//==============================================================================
void Parameters::prepareToPlay(double sampleRate) noexcept
{
    /*          Linear Smoothing
      THe gainSmoother needs to know how long it should take to
      transition from previous parameter value to new one:
        If smoothing time too short short, you'll hear zipper noise
        if smoothing time too long, you';; hear sound fade in or out
        20 ms at 48KHz is 960 samples (a good compromise)
     */
    double duration = 0.02;  // in seconds
    gainSmoother.reset(sampleRate, duration);
    mixSmoother.reset(sampleRate, duration);
    feedbackSmoother.reset(sampleRate, duration);
    stereoSmoother.reset(sampleRate, duration);
    lowCutSmoother.reset(sampleRate, duration);
    highCutSmoother.reset(sampleRate, duration);
    
    /*
        Delay-Line Exponential Transition - filter Coefficient depends on sample rate
        0.2f means 200 ms. After 200ms, the one-pole filter will have approached
        the target value withing 63.2%
        Note: coeff will always be between 1 & 0
     */
    coeff = 1.0f - std::exp(-1.0f / (0.2f * float(sampleRate)));
    
    
}


void Parameters::reset() noexcept
{
    gain = 0.0f;
    gainSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain((gainParam->get())));
    
    mix = 1.0f;  // 100%, mixing in the wet signal fully
    mixSmoother.setCurrentAndTargetValue(mixParam->get() * 0.01f); // Convert Percentage to number;
    
    feedback = 0.0f;  // No feedback
    feedbackSmoother.setCurrentAndTargetValue(feedbackParam->get() * 0.01f);
    
    panL = 0.0f;
    panR = 1.0f;
    stereoSmoother.setCurrentAndTargetValue(stereoParam->get() * 0.01f);
    
    lowCut = 20.0f;
    lowCutSmoother.setCurrentAndTargetValue(lowCutParam->get());
    
    highCut = 20000.0f;
    highCutSmoother.setCurrentAndTargetValue(highCutParam->get());
}
// This function updates the parameters from the latest APTVS source - usally called once per block
void Parameters::update() noexcept
{
    targetDelayTime = delayTimeParam->get();
    if(delayTime == 0.0f)
        delayTime = targetDelayTime;
    /*        The Gain
    float gainInDecibels = gainParam->get(); // reads the current value from the parameter (address in APTVS)
    float newGain = juce::Decibels::decibelsToGain(gainInDecibels);
    gainSmoother.setTargetValue(newGain);// Tells smoother about new value. If it differs, smoother will get to work
     */
    gainSmoother.setTargetValue(juce::Decibels::decibelsToGain(gainParam->get()));
    mixSmoother.setTargetValue(mixParam->get() * 0.01f);
    feedbackSmoother.setTargetValue(feedbackParam->get() * 0.01f);
    stereoSmoother.setTargetValue(stereoParam->get() * 0.01f);
    lowCutSmoother.setTargetValue(lowCutParam->get());
    highCutSmoother.setTargetValue(highCutParam->get());
    
    delayNote = delayNoteParam->getIndex();
    tempoSync = tempoSyncParam->get();
    
    bypassed = bypassParam->get();
}

// Called once per sample
void Parameters::smoothen() noexcept
{
    gain = gainSmoother.getNextValue();
    mix = mixSmoother.getNextValue();
    feedback = feedbackSmoother.getNextValue();
    panningEqualPower(stereoSmoother.getNextValue(), panL, panR);
    lowCut = lowCutSmoother.getNextValue();
    highCut = highCutSmoother.getNextValue();
    
    /*
     Mathematically we want to do: currentValue = currentValue(1 - coff)  + targetValue*coeff
     Work by example, if coeff is 0.1, then currentValue is multiplied by 0.9 and targetValue by 0.1
     The current value gradually becomes less (exponentially decays).At the same time, we add in a small
     portion of the target value. As a result we are always moving towards the target.
     With every timestep the distance between currentValue & targetValue become smaller & the movement slows
     down.Exponential shape: Starts out fast but the steps become smaller & smaller
     */
    //delayTime += (targetDelayTime - delayTime) * coeff;
    delayTime = targetDelayTime;  // Turn off parameter smoothing since it interferes with ducking
}
