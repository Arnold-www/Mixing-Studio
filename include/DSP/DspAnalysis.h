#pragma once

#include <vector>

// Visual/analysis DSP (waveform, VU, spectrum, peak/clip). No Qt dependency.
class DspAnalysis
{
public:
    static std::vector<float> downsampleWaveform(const std::vector<float> &samples, int targetBins);
    static float computeVuLevel(const std::vector<float> &samples);
    // Map peak/RMS to a 0..1 UI meter (approx -42 dBFS .. 0 dBFS).
    static float computeDisplayMeter(float peak, float rms);
    static std::vector<float> computeSpectrumBands(const std::vector<float> &samples, int bandCount);
    static float detectPeak(const std::vector<float> &samples);
    static bool detectClipping(const std::vector<float> &samples, float threshold = 0.99f);
};
