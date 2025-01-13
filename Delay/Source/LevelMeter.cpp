/*
  ==============================================================================

    LevelMeter.cpp

  ==============================================================================
*/

#include "LevelMeter.h"
#include "LookAndFeel.h"

LevelMeter::LevelMeter(Measurement& measurementL_, Measurement& measurementR_)
                    : measurementL(measurementL_), measurementR(measurementR_)
{
    setOpaque(true);
    startTimerHz(refreshRate);
    decay = 1.0f - std::exp(-1.0f/ (float(refreshRate) * 0.2f)); // 0.2f sets decay time to 200ms
}

void LevelMeter::paint(juce::Graphics &g){
    const auto bounds = getLocalBounds();
    g.fillAll(Colors::LevelMeter::background);
    g.setFont(Fonts::getFont(10.0f));
    
    drawLevel(g, dbLevelL, 0, 7);
    drawLevel(g, dbLevelR, 9, 7);
    
    for(float db = maxdB; db >= mindB; db -= stepdB){
        int y = positionForLevel(db);
        
        g.setColour(Colors::LevelMeter::tickLine);
        g.fillRect(0, y, 16, 1);
        
        g.setColour(Colors::LevelMeter::tickLabel);
        g.drawSingleLineText(juce::String(int(db)),
                             bounds.getWidth(), y+3,
                             juce::Justification::right);
    }
}
void LevelMeter::resized(){
    maxPos = 4.0f;
    minPos = float(getHeight()) - 4.0f;
}

void LevelMeter::timerCallback(){
    //DBG("left: " << measurementL.load() << " , right: " << measurementR.load());
    updateLevel(measurementL.readAndReset(), levelL, dbLevelL);
    updateLevel(measurementR.readAndReset(), levelR, dbLevelR); 
    
    // Tells JUCE that painting should happen. JUCE will then perform "pain()"
    // whenever operating system & JUCE determines the compoent will get re-drawn.
    repaint();
}

void LevelMeter::drawLevel(juce::Graphics &g, float level, int x, int width){
    int y = positionForLevel(level);
    if(level > 0.0f){
        int y0 = positionForLevel(0.0f);
        g.setColour(Colors::LevelMeter::tooLoud);
        g.fillRect(x, y, width, y0 - y);
        g.setColour(Colors::LevelMeter::levelOK);
        g.fillRect(x, y0, width, getHeight() - y0);
    }
    else if(y < getHeight()){
        g.setColour(Colors::LevelMeter::levelOK);
        g.fillRect(x, y, width, getHeight() - y);
    }
    // Note: We dont draw anything if the level is too low
}

void LevelMeter::updateLevel(float newLevel, float &smoothedLevel, float &leveldB) const{
    if(newLevel > smoothedLevel)
        smoothedLevel = newLevel;    // instantaneous attack
    else
        smoothedLevel += (newLevel - smoothedLevel)*decay; // exponential fade-out
    
    // In conversion to dB, exponential fade becomes linear fade
    if(smoothedLevel > clampLevel)
        leveldB = juce::Decibels::gainToDecibels(smoothedLevel);
    else
        leveldB = clampdB;
}
