/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.
 
 Video: https://youtu.be/6ZK2Goiyotk?si=ipOl7tYkXx4FoGVN
 GitHub: https://github.com/Signalsmith-Audio/reverb-example-code

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

static constexpr float maxDelayTime = 5000.0f;  // expressed in ms

//==============================================================================
Reverb_XDAudioProcessor::Reverb_XDAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

Reverb_XDAudioProcessor::~Reverb_XDAudioProcessor()
{
}

//==============================================================================
const juce::String Reverb_XDAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Reverb_XDAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Reverb_XDAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Reverb_XDAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Reverb_XDAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Reverb_XDAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Reverb_XDAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Reverb_XDAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Reverb_XDAudioProcessor::getProgramName (int index)
{
    return {};
}

void Reverb_XDAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Reverb_XDAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    
    
    /* JUCE DSP objects must always be prepared before they can be used */
    // DSP objects Need Spec
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = juce::uint32(samplesPerBlock);
    spec.numChannels = 2;
    delay.prepare(spec);
    delay.setMaximumDelayInSamples(maxDelayTime);
    
    delay.reset();
}

void Reverb_XDAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Reverb_XDAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Reverb_XDAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                            [[maybe_unused]]juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    float sampleRate = float(getSampleRate());

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    delaySamples = delayMs * 0.001f * sampleRate;
    delay.setDelay(delaySamples);
    
    float* channelDataL = buffer.getWritePointer(0);
    float* channelDataR = buffer.getWritePointer(1);
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        // Start with reading from the delay line
        float delayedL = delay.popSample(0);
        float delayedR = delay.popSample(1);
        
        // The multiply by gain and add it to the input
        float sumL = channelDataL[sample] + delayedL*decayGain;
        float sumR = channelDataR[sample] + delayedR*decayGain;
        
        // write sum to delay line
        delay.pushSample(0, sumL);
        delay.pushSample(1, sumR);
        
        // update output
        channelDataL[sample] = delayedL;
        channelDataR[sample] = delayedR;
    }
}

//==============================================================================
bool Reverb_XDAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Reverb_XDAudioProcessor::createEditor()
{
    return new Reverb_XDAudioProcessorEditor (*this);
}

//==============================================================================
void Reverb_XDAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Reverb_XDAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Reverb_XDAudioProcessor();
}
