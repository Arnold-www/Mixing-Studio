#pragma once

struct TrackProcessParams
{
    float volume = 0.8f;
    float pan = 0.0f;
    bool audible = true;
    float eqLowDb = 0.0f;
    float eqMidDb = 0.0f;
    float eqHighDb = 0.0f;
    static constexpr int kEqBandCount = 10;
    float eqBands[kEqBandCount] = {};
    bool useGraphicEq = false;
    float compThreshold = 0.7f;
    float compRatio = 4.0f;
    bool fxBypass = false;
};

struct StereoSample
{
    float left = 0.0f;
    float right = 0.0f;
};

// Realtime mix chain: gain, pan, EQ, compressor, linear sum, master limit.
class DspProcessor
{
public:
    static float clampSample(float sample);
    static float applyGain(float sample, float gain);
    static float panLeftGain(float pan);
    static float panRightGain(float pan);
    static float dbToLinear(float gainDb);
    static float applyThreeBandEq(float sample, float lowDb, float midDb, float highDb);
    static float applyGraphicEq(float sample, const float *bands, int bandCount);
    static float applyCompressor(float sample, float threshold, float ratio);
    static StereoSample processTrackSample(float monoSample, const TrackProcessParams &params);
    static StereoSample mixLinear(const StereoSample &accumulator, const StereoSample &track);
    static StereoSample applyMasterChain(const StereoSample &mixed, float masterVolume);
};
