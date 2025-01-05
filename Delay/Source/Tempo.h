/*
  ==============================================================================

    Tempo.h
 
    Handles communication with the host to get additional infomration about:
        - Current tempo
        - Time signature
    from juce::AudioPlayHead object

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Tempo
{
public:
    /**
            Object needs to have a function to reset the internal state when audio playback starts or restarts
     */
    void reset() noexcept;
    
    /**
        Inspect current stae of the host's audio transport. Reads the current tempo
     */
    void update(const juce::AudioPlayHead* playhead) noexcept;
    /**
     Convert a given note lenght, such as 1/4 or 1/2, into a time in milliseconds
     */
    double getMillisecondsforNoteLength(int index) const noexcept;
    /**
     Gets BPM: Describes the tempo in number of quarter notes per minute
     */
    double getTempo() const noexcept{
        return bpm;
    }
    
private:
    double bpm = 120.0; // This should only be changed by Tempo object itself, never by outside code
};
