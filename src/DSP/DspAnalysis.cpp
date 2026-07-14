#include <DSP/DspAnalysis.h>

#include <DSP/DspProcessor.h>

#include <algorithm>
#include <cmath>
#include <vector>

std::vector<float> DspAnalysis::downsampleWaveform(const std::vector<float> &samples, int targetBins)
{
    const int bins = std::max(1, targetBins);
    std::vector<float> output(static_cast<size_t>(bins), 0.0f);
    if (samples.empty()) {
        return output;
    }

    const size_t count = samples.size();
    for (int bin = 0; bin < bins; ++bin) {
        const size_t start = (static_cast<size_t>(bin) * count) / static_cast<size_t>(bins);
        const size_t end = (static_cast<size_t>(bin + 1) * count) / static_cast<size_t>(bins);
        float peak = 0.0f;
        for (size_t i = start; i < end; ++i) {
            peak = std::max(peak, std::fabs(samples[i]));
        }
        float signedPeak = 0.0f;
        for (size_t i = start; i < end; ++i) {
            if (std::fabs(samples[i]) >= peak - 1e-6f) {
                signedPeak = samples[i];
                break;
            }
        }
        output[static_cast<size_t>(bin)] = DspProcessor::clampSample(signedPeak);
    }
    return output;
}

float DspAnalysis::computeVuLevel(const std::vector<float> &samples)
{
    if (samples.empty()) {
        return 0.0f;
    }

    double sumSquares = 0.0;
    for (float sample : samples) {
        sumSquares += static_cast<double>(sample) * static_cast<double>(sample);
    }
    const float rms = static_cast<float>(std::sqrt(sumSquares / static_cast<double>(samples.size())));
    return std::clamp(rms, 0.0f, 1.0f);
}

std::vector<float> DspAnalysis::computeSpectrumBands(const std::vector<float> &samples, int bandCount)
{
    const int bands = std::max(1, bandCount);
    std::vector<float> levels(static_cast<size_t>(bands), 0.0f);
    if (samples.empty()) {
        return levels;
    }

    const int n = static_cast<int>(samples.size());
    const int half = n / 2;
    if (half <= 0) {
        return levels;
    }

    std::vector<float> magnitudes(static_cast<size_t>(half), 0.0f);
    for (int k = 0; k < half; ++k) {
        double real = 0.0;
        double imag = 0.0;
        const double omega = 2.0 * 3.14159265358979323846 * static_cast<double>(k) / static_cast<double>(n);
        for (int t = 0; t < n; ++t) {
            const double angle = omega * static_cast<double>(t);
            real += static_cast<double>(samples[static_cast<size_t>(t)]) * std::cos(angle);
            imag -= static_cast<double>(samples[static_cast<size_t>(t)]) * std::sin(angle);
        }
        magnitudes[static_cast<size_t>(k)] =
            static_cast<float>(std::sqrt(real * real + imag * imag) / static_cast<double>(n));
    }

    float maxMag = 0.0f;
    for (int band = 0; band < bands; ++band) {
        const int start = (band * half) / bands;
        const int end = ((band + 1) * half) / bands;
        float peak = 0.0f;
        for (int i = start; i < end; ++i) {
            peak = std::max(peak, magnitudes[static_cast<size_t>(i)]);
        }
        levels[static_cast<size_t>(band)] = peak;
        maxMag = std::max(maxMag, peak);
    }

    if (maxMag > 1e-6f) {
        for (float &level : levels) {
            level = std::clamp(level / maxMag, 0.0f, 1.0f);
        }
    }

    return levels;
}

float DspAnalysis::detectPeak(const std::vector<float> &samples)
{
    float peak = 0.0f;
    for (float sample : samples) {
        peak = std::max(peak, std::fabs(sample));
    }
    return std::clamp(peak, 0.0f, 1.0f);
}

bool DspAnalysis::detectClipping(const std::vector<float> &samples, float threshold)
{
    const float limit = std::clamp(threshold, 0.5f, 1.0f);
    for (float sample : samples) {
        if (std::fabs(sample) >= limit) {
            return true;
        }
    }
    return false;
}
