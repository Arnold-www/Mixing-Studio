#include <DSP/DspProcessor.h>

#include <algorithm>

float DspProcessor::clampSample(float sample)
{
    return std::clamp(sample, -1.0f, 1.0f);
}

float DspProcessor::applyGain(float sample, float gain)
{
    return clampSample(sample * gain);
}

float DspProcessor::panLeftGain(float pan)
{
    const float normalizedPan = std::clamp(pan, -1.0f, 1.0f);
    return normalizedPan <= 0.0f ? 1.0f : 1.0f - normalizedPan;
}

float DspProcessor::panRightGain(float pan)
{
    const float normalizedPan = std::clamp(pan, -1.0f, 1.0f);
    return normalizedPan >= 0.0f ? 1.0f : 1.0f + normalizedPan;
}
