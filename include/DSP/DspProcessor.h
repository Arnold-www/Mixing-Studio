#pragma once

#include <QtGlobal>

class DspProcessor
{
public:
    static float clampSample(float sample);
    static float applyGain(float sample, float gain);
    static float panLeftGain(float pan);
    static float panRightGain(float pan);
};
