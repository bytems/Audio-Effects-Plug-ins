/*
  ==============================================================================

    DelayLine.cpp
 DelayLine class that is user's own implementation of a circular buffer,

  ==============================================================================
*/

#include <JuceHeader.h>
#include "DelayLine.h"

void DelayLine::setMaximumDelayInSamples(int maxLengthInSamples){
    jassert(maxLengthInSamples > 0);
    int paddedLength = maxLengthInSamples + 1; // If buffer was 5 samples, max delay would be 4
    if(bufferLength < paddedLength){
        bufferLength = paddedLength;
        buffer.reset(new float[size_t(bufferLength)]);
    }
}

// Clear out old data from the delay line
void DelayLine::reset() noexcept{
    writeIndex = bufferLength - 1;
    for(size_t i = 0; i < size_t(bufferLength); ++i)
        buffer[i] = 0.0;
}

void DelayLine::write(float sample) noexcept{
    jassert(bufferLength > 0);
    writeIndex = (writeIndex + 1) % bufferLength;
    buffer[writeIndex] = sample;
}

/*          Nearest neibhboring sample approach
float DelayLine::read(float delayInSamples) const noexcept{
    jassert(delayInSamples >= 0.0f);
    jassert(delayInSamples < bufferLength);
    int readIndex = std::round(writeIndex - delayInSamples);  // Nearest neighbor interpolation
    if(readIndex < 0)
        readIndex += bufferLength;
    return buffer[size_t(readIndex)];
}
*/

// Linear interpolation approach
float DelayLine::read(float delayInSamples) const noexcept{
    jassert(delayInSamples >= 0.0f);
    jassert(delayInSamples <= bufferLength - 1.0f);
    
    int integer_delay = int(delayInSamples); // Strips out fractional component
    float fraction = delayInSamples - float(integer_delay);
    
    int readIndexA = writeIndex - integer_delay;
    int readIndexB = writeIndex - integer_delay - 1;
    
    if(readIndexA < 0) readIndexA += bufferLength;
    if(readIndexB < 0) readIndexB += bufferLength;
    
    float sampleA = buffer[readIndexA];
    float sampleB = buffer[readIndexB];
    
    return sampleA + fraction * (sampleB - sampleA);
}

/* Hermite (4 pts) Interpolation
float DelayLine::read(float delayInSamples) const noexcept{
    jassert(delayInSamples >= 1.0f);
    jassert(delayInSamples <= bufferLength - 2.0f);
    
    int integerDelay = int(delayInSamples);
    
    // Get the 4 indices
    // 2 samples to the right
    int readIndexA = writeIndex - integerDelay + 1;
    int readIndexB = readIndexA - 1;
    // 2 samples to the left
    int readIndexC = readIndexA - 2;
    int readIndexD = readIndexA - 3;
    
    //   Wrap-around indices
    // Point D is the smallest index. If its not negative none of the points need to wrapped around
    if(readIndexD < 0){
        readIndexD += bufferLength;
        if(readIndexC < 0){
            readIndexC += bufferLength;
            if(readIndexB < 0){
                readIndexB += bufferLength;
                if(readIndexA < 0){
                    readIndexA += bufferLength;
                }
            }
        }
    }
    
    // Get the 4 samples
    float sampleA = buffer[size_t(readIndexA)];
    float sampleB = buffer[size_t(readIndexB)];
    float sampleC = buffer[size_t(readIndexC)];
    float sampleD = buffer[size_t(readIndexD)];
    
    // Create the curve throug the 4 samples and find the interpolated value
    float fraction = delayInSamples - float(integerDelay);
    float slope0 = (sampleC - sampleA) * 0.5f;
    float slope1 = (sampleD - sampleB) * 0.5f;
    float v = sampleB - sampleC;
    float w = slope0 + v;
    float a = w + v + slope1;
    float b = w + a;
    float stage1 = a * fraction - b;
    float stage2 = stage1 * fraction + slope0;
    return stage2 * fraction + sampleB;
}
*/
