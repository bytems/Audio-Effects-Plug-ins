/*
  ==============================================================================

    DSP.h
    Little library of DSP code

  ==============================================================================
*/

#pragma once

#include <cmath>   // for cos & sin

/**
   The equal power panning law ensures consistent perceived loudness when audio is panned between two
   channels.It adjusts the gain of each channel such that the total power remains constant across the
   stereo field. This is achieved by applying a sine and cosine law:
   Left Gain = cos(theta),
   Right Gain = sin(theta)
   where  theta  is the pan angle (0° = center, ± 0.25 pi = fully left/right).
   This avoids perceived loudness dips in the center.
 
 @param panning value between -1 & 1. -1 means sound is panned fully left.
 
 */
inline void panningEqualPower(float panning, float &left, float& right)
{
    // pi / 4 = 0.785....
    float theta = 0.7853981633974483f * (panning + 1.0f);
    left = std::cos(theta);
    right = std::sin(theta);
}
