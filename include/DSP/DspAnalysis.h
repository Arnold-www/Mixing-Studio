#pragma once

#include <vector>

// Visual/analysis DSP (waveform, VU, spectrum, peak/clip). No Qt dependency.
class DspAnalysis
{
public:
    static std::vector<float> downsampleWaveform(const std::vector<float> &samples, int targetBins);
    static float computeVuLevel(const std::vector<float> &samples);
    static std::vector<float> computeSpectrumBands(const std::vector<float> &samples, int bandCount);
    static float detectPeak(const std::vector<float> &samples);
    static bool detectClipping(const std::vector<float> &samples, float threshold = 0.99f);
};
