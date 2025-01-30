/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ProtectYourEars.h"

//==============================================================================
DelayAudioProcessor::DelayAudioProcessor() 
    : AudioProcessor(
                     BusesProperties()
                        .withInput("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput("Output", juce::AudioChannelSet::stereo(),true)
      ),
    params(apvts)
{
    lowCutFilter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    highCutFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
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

void DelayAudioProcessor::setCurrentProgram (int)
{
}

const juce::String DelayAudioProcessor::getProgramName (int)
{
    return {};
}

void DelayAudioProcessor::changeProgramName (int, const juce::String&)
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
    tempo.reset();
    
    /* JUCE DSP objects must always be prepared before they can be used */
    // DSP objects Need Spec
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = juce::uint32(samplesPerBlock);
    spec.numChannels = 2;
    //delayLine.prepare(spec);
    lowCutFilter.prepare(spec);
    highCutFilter.prepare(spec);
    
    // DelayLine
    double numSamples = Parameters::maxDelayTime / 1000.0 * sampleRate;
    int maxDelayInSamples = int(std::ceil(numSamples));
    delayLineL.setMaximumDelayInSamples(maxDelayInSamples);
    delayLineR.setMaximumDelayInSamples(maxDelayInSamples);
    
    /*         Reset all params & variables        */
    
    // Delay Line Params
    delayInSamples = 0.0f;
    targetDelay = 0.0f;
    
    // fading applied to feedback
    fade = 1.0;          // Current wet signal envelope level
    fadeTarget = 1.0f;
    
    coeff = 1.0 - std::exp(-1.0f /(0.05 * float(sampleRate)));
    
    // waiting variables determine how long to hold ducking until fading back in
    wait = 0.0f;
    waitInc = 1.0 / (0.3f * float(sampleRate)); // 300 ms. At 48 kHz, 0.3 * 48k = 14,400 samples.
                                                // 300ms corresponds to 14,400 timesteps
    
    // Clear out any old sample values from the stereo feedback path
    feedbackL = 0.0f;
    feedbackR = 0.0f;
    
    // Audio Level Meters
    levelL.reset();
    levelR.reset();
    
    delayLineL.reset();
    delayLineR.reset();
    lowCutFilter.reset();
    highCutFilter.reset();
}

void DelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

/*
  When the plug-in is loaded, the host will repeatedly call this function with different options
  to see which buses the plug-in supports. Allows host & plug-in to negotiate best bus for a situation
 */
bool DelayAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mono =   juce::AudioChannelSet::mono();
    const auto stereo = juce::AudioChannelSet::stereo();
    const auto mainIn =  layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();
        
    if(mainIn == mono   && mainOut == mono)   return true;    // mono -> mono
    if(mainIn == mono   && mainOut == stereo) return true;    // mono -> stereo
    if(mainIn == stereo && mainOut == stereo) return true;   // stereo -> stereo
    // return mainOut == stereo;
    return false;
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

    /* This is the place where you'd normally do the guts of your plugin's audio processing...  */
    params.update(); // reads the most recent parameter values, updating target value of any smoothers
    tempo.update(getPlayHead());
    float syncedTime = std::min<float>(tempo.getMillisecondsforNoteLength(params.delayNote),
                                       Parameters::maxDelayTime);
    float maxL = 0.0f; // Used to measure peak level for current block
    float maxR = 0.0f;
    
    /** @param buffer: Contains channels for all input buses & output buses. Sadly, it does not make a distinction
                       between the number of input channels vs number of output channels.
     */
    // Get read access to input bus chaneels
    auto mainInput = getBusBuffer(buffer, true, 0);
    auto mainInputChannels = mainInput.getNumChannels();
    auto isMainInputStereo = mainInputChannels > 1;
    const float* inputDataL = mainInput.getReadPointer(0);
    const float* inputDataR = mainInput.getReadPointer(isMainInputStereo ? 1 : 0);
    
    // Get write access to output bus channels
    auto mainOutput = getBusBuffer(buffer, false, 0);
    auto mainOutputChannesl = mainOutput.getNumChannels();
    auto isMainOutputStereo = mainOutputChannesl > 1;
    float* outputDataL = mainOutput.getWritePointer(0);
    float* outputDataR = mainOutput.getWritePointer(isMainOutputStereo ? 1 : 0);
    
    /*        Processing Loop          */
    if(isMainInputStereo){  //  Stereo Audio processing loop
        for(int sample = 0; sample < buffer.getNumSamples(); sample++){
            params.smoothen();  // Smooth motion prevents zipper noise
            
            // Update Delay Line
            float delayTime = params.tempoSync ? syncedTime : params.delayTime;
            float newTargetDelay = delayTime / 1000.0f * sampleRate;
            
            // Decide whether to perform ducking
            if(newTargetDelay != targetDelay){
                targetDelay = newTargetDelay;
                if(delayInSamples == 0.0f)  // first time
                    delayInSamples = targetDelay;
                else{ // start fading out & reset wait period
                    wait       = waitInc; // start counter
                    fadeTarget = 0.0;  // Initiates fade out & activates one-pole filter
                }
                
            }
            
            // Update SVF filters
            if(params.lowCut != lastLowCut) // Only update/modify filter if Cut freq changed from last time
            {
                lowCutFilter.setCutoffFrequency(params.lowCut);
                lastLowCut = params.lowCut;
            }
            if(params.highCut != lastHighCut){
                highCutFilter.setCutoffFrequency(params.highCut);
                lastHighCut = params.highCut;
            }
            
            
            // WE need to proc L & R channel at the same time so that the same smoothed param is applied
            
            float dryL = inputDataL[sample];    // Dry sample: What we call the unprocessed audio
            float dryR = inputDataR[sample];
            
            // convert stereo to mono
            float mono = (dryL + dryR) * 0.5f;
            
            // Add the sample coming from the feedback path to the dry signal, and put sum in delay line
            // push the mono signal into the delay line
            // Ping-Poing feedback: Notice we are feedback R to the left channels delay line
            delayLineL.write(mono*params.panL + feedbackR);
            delayLineR.write(mono*params.panR + feedbackL);
            
            // Wet sample: What we call processed signals
            float wetL = delayLineL.read(delayInSamples);
            float wetR = delayLineR.read(delayInSamples);
            
            /* Slowly & smoothly move the value of fade towards fadeTarget
               Only happens while ducking, otherwise fade stays same value
             */
            fade += (fadeTarget - fade) * coeff;   // one-pole filter formula.
            
            /* Apply fade as envelope of wet signal.
             Most of the time fade = 1, and nothing happens to delayed sound
             However, when we're ducking, the wet signal is suppressed
            */
            wetL *= fade;
            wetR *= fade;
            
            if(wait > 0.0f){
                wait += waitInc;
                if(wait >= 1.0f){
                    // Holding period is over. Switch to new delay length and start fading it in
                    delayInSamples = targetDelay;
                    wait = 0.0f;
                    fadeTarget = 1.0f; // fade in
                }
            }
            
            /* Read output from delay line, and:
                -apply low/high-cut filters
                -apply feedback gain to get new feedback sample
              Note that what we're writing to feedback isnt used until next iteration of loop.
             */
            feedbackL = wetL * params.feedback;
            feedbackL =  lowCutFilter.processSample(0, feedbackL);
            feedbackL = highCutFilter.processSample(0, feedbackL);
            
            feedbackR = wetR * params.feedback;
            feedbackR =  lowCutFilter.processSample(1, feedbackR);
            feedbackR = highCutFilter.processSample(1, feedbackR);
            
            // Create mix. Mixing the processed audio with the original dry sound is called the dry/wet mix
            float mixL = dryL + wetL * params.mix;
            float mixR = dryR + wetR * params.mix;
            
            // Apply the final gain
            float outL = mixL * params.gain;
            float outR = mixR * params.gain;
            
            /* In Bypass Mode, we still need all the calcualations to create the wet signal to maintain state
               However, we only output the dry signal
             */
            if(params.bypassed){
                outL = dryL;
                outR = dryR;
            }
            
            // Write the output samples back into the juce::AudioBuffer
            outputDataL[sample] = outL;
            outputDataR[sample] = outR;
            
            // Atmomically store the peaks (will be communicated to Editor)
            maxL = std::max(maxL, std::abs(outL));
            maxR = std::max(maxR, std::abs(outR));
            
            /*
                Outputting values as if they were an audio signal and looking at them using the oscilloscope
                is a simple trick to help debug the plug-in and verify everything works as it should.
             */
            //outputDataL[sample] = params.delayTime / 5000.0f;  // Checks to see how delay time smoothly
            //outputDataR[sample] = params.delayTime/ 5000.0f;   // transitions
            //outputDataL[sample] = fade;     // CHeck to see how delay Line fades
            //outputDataR[sample] = fade;
        }
        
        levelL.updateIfGreater(maxL);
        levelR.updateIfGreater(maxR);
        
        #if JUCE_DEBUG
        protectYourEars(buffer);  // Techniacally not allowed in audio thread (its slow w system calls)
                                  // However statement prints something in an exceptional situation, so it OK
        #endif
    }
    else { // Processing loop for mono
        for(int sample = 0; sample < buffer.getNumSamples(); ++sample){
            params.smoothen();
            
            float delayInSamples = params.delayTime / 1000.0f * sampleRate;
            
            float dry = inputDataL[sample];
            delayLineL.write(dry + feedbackL);
            
            float wet = delayLineL.read(delayInSamples);
            feedbackL = wet * params.feedback;
            
            float mix = dry + wet*params.mix;
            outputDataL[sample] = mix * params.gain;
        }
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
juce::AudioProcessorParameter* DelayAudioProcessor::getBypassParameter() const{
    return params.bypassParam;
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
