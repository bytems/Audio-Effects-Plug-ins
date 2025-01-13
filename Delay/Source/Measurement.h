/*
  ==============================================================================

    Measurement.h
    Helper object that uses atomics for communication between Editor & Processor
    regarding Level Meter
        - Keeps track of the highest level (value) recorded, until we get a reset
        - Reset occurs when Editor has read & displayed the measurement
 
    Note: Lock-Free data structure

  ==============================================================================
*/

#pragma once

#include <atomic>

struct Measurement
{
    std::atomic<float> value;

    void reset() noexcept{
        value.store(0.0f);
    }
    
    /*
        Utilized by Audio (real-time) thread
     */
    void updateIfGreater(float newValue) noexcept{
        auto oldValue = value.load();
        // A CAS loop ensures that newValue is only placed into the atomic
        // if its larger than the atomic's current value
        while(newValue > oldValue &&
              !value.compare_exchange_weak(oldValue, newValue));
    }

    /*
        Utilzed by UI thread
     */
    float readAndReset() noexcept{
       return value.exchange(0.0f); // Setting the atmoic back to 0.0f signals to the audio thread that it is
                                    //  OK to write smaller measurements again in the next process block
        
    }
};
