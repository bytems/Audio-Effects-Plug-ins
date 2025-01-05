/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Parameters.h" // for Plug-in Parameters
#include "Tempo.h"
#include "DelayLine.h"


//==============================================================================
/**
*/
class DelayAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    DelayAudioProcessor();
    ~DelayAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    // Host(DAW) transports the audio data to & from the plug-in over a "bus".
    // Leftover term from mixing consoles & how audi is routed between different parts of mixing console
   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    /* This is where "DSP" Digital Signal Processing happens. Plug-in's audio processing code
     * Host will call this 100 - 1000 times a second
     */
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override; // Host (aka DAW) calls this function to open UI
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    /* Used by the host to save save & restore the plug-in's paramter values.
     * Allows user to save DAW session & then load it at a later time.
     */
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    juce::AudioProcessorValueTreeState apvts{  // Variable named apvts, which will manage plug-in parameters
        *this, // connects APTVS to this plug-in
        nullptr, // Undo Manager -- ignore
        "Parameters",  // Name for apvts
        Parameters::createParameterLayout() // list of parameters the plug-in has
    };
    
    Parameters params; // Tells the DelayAudioProc that it has Parameters object

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayAudioProcessor)
    
    Tempo tempo;
    
    // DelayLine: Delay sound by a certain amount of time. We keep track of samples
    // in Juce's own Circular buffer. A chunk of memory that stores samples
    // & waits for the right moment to start outputting them
    //juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine;
    DelayLine delayLineL, delayLineR;
    
    // Stereo Feedback state
    float feedbackL = 0.0f;
    float feedbackR = 0.0f;
    
    // SVF from JUCE
    juce::dsp::StateVariableTPTFilter<float> lowCutFilter;
    juce::dsp::StateVariableTPTFilter<float> highCutFilter;
    
    // Previous param state
    float lastLowCut = -1.0f;
    float lastHighCut = -1.0f;   // "no cutoff frequency set yet"
};
