/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayAudioProcessor::DelayAudioProcessor() 
    : AudioProcessor(
                     BusesProperties()
                        .withInput("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput("Output", juce::AudioChannelSet::stereo(),true)
      ),
    params(apvts)
{
    // do nothing
}

DelayAudioProcessor::~DelayAudioProcessor()
{
}

//==============================================================================
const juce::String DelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void DelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
// This is the plug-in’s chance to get everything ready to go before it starts receiving audio.
// Prepare all internal resources
void DelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback initialisation that you need..
    
    // Prepare all parameter supporters
    params.prepareToPlay(sampleRate);
    params.reset();
    
    // Prepare DelayLine  - Need Spec & MaxDelay
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = juce::uint32(samplesPerBlock);
    spec.numChannels = 2;
    delayLine.prepare(spec);
    
    double numSamples = Parameters::maxDelayTime / 1000.0 * sampleRate;
    int maxDelayInSamples = int(std::ceil(numSamples));
    delayLine.setMaximumDelayInSamples(maxDelayInSamples);
    delayLine.reset();
    //DBG(maxDelayInSamples);
}

void DelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool DelayAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void DelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, [[maybe_unused]] juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    float sampleRate = getSampleRate();
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output channels that didn't contain input data,
    //(because these aren't guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback when they first compile a plugin,
    // but obviously you don't need to keep  this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's audio processing...
    // Make sure to reset the state if your inner loop is processing the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels interleaved by keeping the same state.
    
    params.update(); // reads the most recent parameter values, updating target value of any smoothers
    
    //  Two ways to apply gain - Use juce::AudioBuffer function
    //buffer.applyGain(0.1f);
    // Manually
    float* channelDataL = buffer.getWritePointer(0);
    float* channelDataR = buffer.getWritePointer(1);
    for(int sample = 0; sample < buffer.getNumSamples(); sample++){ //  loop handling the samples
        params.smoothen();  // Smooth motion prevents zipper noise
        
        float delayInSamples = params.delayTime / 1000.0f * sampleRate;
        delayLine.setDelay(delayInSamples);
        
        // WE need to proc L & R channel at the same time so that the same smoothed param is applied
        
        float dryL = channelDataL[sample];    // Dry sample: What we call the unprocessed audio
        float dryR = channelDataR[sample];
        delayLine.pushSample(0, dryL);
        delayLine.pushSample(1, dryR);
        float wetL = params.mix * delayLine.popSample(0);  // Wet sample: What we call processed signals
        float wetR = params.mix * delayLine.popSample(1);
        
        // Mixing the delayed audio with the original dry sound is called the dry/wet mix
        channelDataL[sample] = (wetL + dryL) * params.gain;
        channelDataR[sample] = (wetR + dryR) * params.gain;
        
        /*
            Outputting values as if they were an audio signal and looking at them using the oscilloscope
            is a simple trick to help debug the plug-in and verify everything works as it should.
         */
        //channelDataL[sample] = params.delayTime / 5000.0f;  // Checks to see how delay time smoothly
        //channelDataR[sample] = params.delayTime/ 5000.0f;   // transitions
    }
}

//==============================================================================
bool DelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

// DAW
juce::AudioProcessorEditor* DelayAudioProcessor::createEditor()
{
    return new DelayAudioProcessorEditor (*this);
}

//==============================================================================
/*
    Serialize (save) the plug-in's state in the given juce::MemoryBlock
    Serialize: Fancy word for putting data into a format which can be stored in a file.
               Serialiazation format is encoded XML
 */
void DelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    copyXmlToBinary(*apvts.copyState().createXml(), destData);
    //DBG(apvts.copyState().toXmlString());
}

/*
    Deserialize (load) the plug-in's state
 */
void DelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if(xml.get() != nullptr && xml->hasTagName(apvts.state.getType())){
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DelayAudioProcessor();
}
