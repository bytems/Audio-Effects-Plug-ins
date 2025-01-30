/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
                        Reverb!!
 Two basic building blocks
 
             ---------            ------------
      x     |         |          |            |     y
   ------>  |  Delay  |  ------> |Mixing Mtrx |  ------->
            |         |          |            |
             ---------            ------------
 
 An effect which creates a sense of space , by generating a diffuse longer-lasting sound from a short input
*/
class Reverb_XDAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Reverb_XDAudioProcessor();
    ~Reverb_XDAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
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
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    float delayMs = 80.0f;
    float decayGain = 0.85f;
    float delaySamples;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delay;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Reverb_XDAudioProcessor)
};
