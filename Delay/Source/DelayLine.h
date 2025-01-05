/*
  ==============================================================================

    DelayLine.h
    DelayLine class that is user's own implementation of a circular buffer,
    very similar to juce::dsp::DelayLine but there will also be some differences.
    The most important difference is that there is no specific function to set
    the delay. Rather, delay length is specified when "read" is called


  ==============================================================================
*/

#pragma once

#include <memory>

class DelayLine
{
public:
    /** Allocates the memory used to store the contents of the delay line.
        This should be called prior to any processing, from prepareToPlay.
     */
    void setMaximumDelayInSamples(int maxLengthInSamples);
    
    /** Clears the delay line and resets all state. This should be called before first usage. */
    void reset() noexcept;
    
    /** Returns the maximum delay length in samples. */
    int getBufferLength() noexcept{
        return bufferLength;
    }
    
    /** Places a new sample into the delay line, overwriting the previous oldest element. Does the same as JUCE’s pushSample. */
    void write(float sample) noexcept;
    
    /** Reads a sample from the delay line, similar to JUCE’s popSample. */
    float read(float delayInSamples) const noexcept;
    
private:
    std::unique_ptr<float[]> buffer; // Holds the memory region that will store the delayed samples
    int bufferLength = 0;
    int writeIndex = 0; // where the most recent value was written
};
