#include <DSP/DspProcessor.h>

#include <algorithm>
#include <cmath>

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

float DspProcessor::dbToLinear(float gainDb)
{
    const float clampedDb = std::clamp(gainDb, -24.0f, 24.0f);
    return std::pow(10.0f, clampedDb / 20.0f);
}

float DspProcessor::applyThreeBandEq(float sample, float lowDb, float midDb, float highDb)
{
    const float gain = dbToLinear(lowDb) * dbToLinear(midDb) * dbToLinear(highDb);
    return applyGain(sample, gain);
}

float DspProcessor::applyGraphicEq(float sample, const float *bands, int bandCount)
{
    if (!bands || bandCount <= 0) {
        return clampSample(sample);
    }

    float gain = 1.0f;
    for (int i = 0; i < bandCount; ++i) {
        gain *= dbToLinear(bands[i]);
    }
    return applyGain(sample, gain);
}

float DspProcessor::applyCompressor(float sample, float threshold, float ratio)
{
    const float clampedThreshold = std::clamp(threshold, 0.05f, 1.0f);
    const float clampedRatio = std::max(1.0f, ratio);
    const float magnitude = std::fabs(sample);
    if (magnitude <= clampedThreshold) {
        return clampSample(sample);
    }

    const float overshoot = magnitude - clampedThreshold;
    const float compressed = clampedThreshold + (overshoot / clampedRatio);
    return clampSample(std::copysign(compressed, sample));
}

StereoSample DspProcessor::processTrackSample(float monoSample, const TrackProcessParams &params)
{
    StereoSample output;
    if (!params.audible) {
        return output;
    }

    float processed = clampSample(monoSample);
    if (!params.fxBypass) {
        if (params.useGraphicEq) {
            processed = applyGraphicEq(processed, params.eqBands, TrackProcessParams::kEqBandCount);
        } else {
            processed = applyThreeBandEq(processed, params.eqLowDb, params.eqMidDb, params.eqHighDb);
        }
        processed = applyCompressor(processed, params.compThreshold, params.compRatio);
    }

    processed = applyGain(processed, std::clamp(params.volume, 0.0f, 1.0f));
    output.left = clampSample(processed * panLeftGain(params.pan));
    output.right = clampSample(processed * panRightGain(params.pan));
    return output;
}

StereoSample DspProcessor::mixLinear(const StereoSample &accumulator, const StereoSample &track)
{
    StereoSample mixed;
    mixed.left = accumulator.left + track.left;
    mixed.right = accumulator.right + track.right;
    return mixed;
}

StereoSample DspProcessor::applyMasterChain(const StereoSample &mixed, float masterVolume)
{
    StereoSample mastered;
    const float gain = std::clamp(masterVolume, 0.0f, 1.0f);
    mastered.left = applyGain(mixed.left, gain);
    mastered.right = applyGain(mixed.right, gain);
    mastered.left = clampSample(mastered.left);
    mastered.right = clampSample(mastered.right);
    return mastered;
}
