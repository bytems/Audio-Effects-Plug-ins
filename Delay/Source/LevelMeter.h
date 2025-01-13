/*
  ==============================================================================

    LevelMeter.h
    UI component class that provides:
        -
        - Polling of peak levels from Audio Thread

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Measurement.h"

class LevelMeter : public juce::Component, private juce::Timer
{
public:
    LevelMeter(Measurement& measurementL, Measurement& measurementR);
    ~LevelMeter() override{}
    
    void paint(juce::Graphics&) override;
    void resized() override;
    
private:
    /**
        Function is called 60 times per sec by the timer. It will read peak levels & re-draw the component
     */
    void timerCallback() override;
    
    /** Converts (maps)  a dB value to a pixel postion  */
    int positionForLevel(float dbLevel) const noexcept
    {
        return int(std::round(juce::jmap(dbLevel,
                                         maxdB,  mindB,
                                         maxPos, minPos)));
    }
    
    /**
        Paints a vertical bar representing the level for a single channel
     @param x determines where bar will be drawn
     @param width determines how wide bar will be
     */
    void drawLevel(juce::Graphics& g, float level, int x, int width);
    
    /** Apply One-pole filter for smooth decay
        When new measurment is lower, we want the meter to smoothly decay.
         aka Decay animation
     */
    void updateLevel(float newLevel, float& smoothedLevel, float& leveldB) const;
    
    // References to the atomics presumably from the Audio Processor (real-time thread)
    Measurement& measurementL;
    Measurement& measurementR;
    
    // Constants for the drawing
    static constexpr float maxdB = 6.0f;
    static constexpr float mindB = -60.0f;
    static constexpr float stepdB = 6.0f;
    
    static constexpr float clampdB = -120.0f;
    static constexpr float clampLevel = 0.000001f;  // -120 dB
    
    static constexpr int refreshRate = 60;
    
    // Levels read
    float dbLevelL = clampdB;
    float dbLevelR = clampdB;
    
    // Decay variables
    float decay = 0.0f;
    float levelL = clampLevel;
    float levelR = clampLevel;
    
    // Y-coordinates for pixel positions of maxdB & mindB
    float maxPos = 0.0f;
    float minPos = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};
