#include <DSP/DspAnalysis.h>

#include <DSP/DspProcessor.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace {

int nextPowerOfTwo(int value)
{
    int n = 1;
    while (n < value) {
        n <<= 1;
    }
    return n;
}

void fftInPlace(std::vector<float> &real, std::vector<float> &imag)
{
    const int n = static_cast<int>(real.size());
    if (n <= 1) {
        return;
    }

    int j = 0;
    for (int i = 1; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(real[static_cast<size_t>(i)], real[static_cast<size_t>(j)]);
            std::swap(imag[static_cast<size_t>(i)], imag[static_cast<size_t>(j)]);
        }
    }

    for (int len = 2; len <= n; len <<= 1) {
        const double angle = -2.0 * 3.14159265358979323846 / static_cast<double>(len);
        const double wLenReal = std::cos(angle);
        const double wLenImag = std::sin(angle);
        for (int i = 0; i < n; i += len) {
            double wReal = 1.0;
            double wImag = 0.0;
            for (int k = 0; k < len / 2; ++k) {
                const int u = i + k;
                const int v = i + k + len / 2;
                const double tReal = wReal * real[static_cast<size_t>(v)] - wImag * imag[static_cast<size_t>(v)];
                const double tImag = wReal * imag[static_cast<size_t>(v)] + wImag * real[static_cast<size_t>(v)];
                real[static_cast<size_t>(v)] = static_cast<float>(real[static_cast<size_t>(u)] - tReal);
                imag[static_cast<size_t>(v)] = static_cast<float>(imag[static_cast<size_t>(u)] - tImag);
                real[static_cast<size_t>(u)] = static_cast<float>(real[static_cast<size_t>(u)] + tReal);
                imag[static_cast<size_t>(u)] = static_cast<float>(imag[static_cast<size_t>(u)] + tImag);
                const double nextWReal = wReal * wLenReal - wImag * wLenImag;
                wImag = wReal * wLenImag + wImag * wLenReal;
                wReal = nextWReal;
            }
        }
    }
}

} // namespace

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
        const size_t end = std::max(start + 1, (static_cast<size_t>(bin + 1) * count) / static_cast<size_t>(bins));
        float peakAbs = 0.0f;
        float signedPeak = 0.0f;
        for (size_t i = start; i < end && i < count; ++i) {
            const float sample = samples[i];
            const float magnitude = std::fabs(sample);
            if (magnitude >= peakAbs) {
                peakAbs = magnitude;
                signedPeak = sample;
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

float DspAnalysis::computeDisplayMeter(float peak, float rms)
{
    // Prefer peak; lift RMS toward peak so sustained tones still read strongly.
    const float linear = std::max(std::clamp(peak, 0.0f, 1.0f),
                                  std::clamp(rms, 0.0f, 1.0f) * 1.41421356f);
    if (linear <= 1.0e-5f) {
        return 0.0f;
    }

    // Linear amplitude under-reads on screen; use a short dB window for UI.
    const float db = 20.0f * std::log10(std::max(linear, 1.0e-5f));
    constexpr float kFloorDb = -42.0f;
    return std::clamp((db - kFloorDb) / (-kFloorDb), 0.0f, 1.0f);
}

std::vector<float> DspAnalysis::computeSpectrumBands(const std::vector<float> &samples, int bandCount)
{
    const int bands = std::max(1, bandCount);
    std::vector<float> levels(static_cast<size_t>(bands), 0.0f);
    if (samples.empty()) {
        return levels;
    }

    const int n = nextPowerOfTwo(static_cast<int>(samples.size()));
    std::vector<float> real(static_cast<size_t>(n), 0.0f);
    std::vector<float> imag(static_cast<size_t>(n), 0.0f);
    const size_t copyCount = std::min(samples.size(), static_cast<size_t>(n));
    for (size_t i = 0; i < copyCount; ++i) {
        // Hann window reduces spectral leakage for UI meters.
        const float window = 0.5f
            * (1.0f - std::cos(2.0f * 3.14159265f * static_cast<float>(i) / static_cast<float>(copyCount)));
        real[i] = samples[i] * window;
    }

    fftInPlace(real, imag);

    const int half = n / 2;
    if (half <= 0) {
        return levels;
    }

    std::vector<float> magnitudes(static_cast<size_t>(half), 0.0f);
    float maxMag = 0.0f;
    for (int k = 0; k < half; ++k) {
        const float mag = std::sqrt(real[static_cast<size_t>(k)] * real[static_cast<size_t>(k)]
                                    + imag[static_cast<size_t>(k)] * imag[static_cast<size_t>(k)]);
        magnitudes[static_cast<size_t>(k)] = mag;
        maxMag = std::max(maxMag, mag);
    }

    for (int band = 0; band < bands; ++band) {
        // Log-ish spacing: more resolution in lower frequencies.
        const float t0 = static_cast<float>(band) / static_cast<float>(bands);
        const float t1 = static_cast<float>(band + 1) / static_cast<float>(bands);
        const int start = std::max(1, static_cast<int>(std::pow(static_cast<float>(half - 1), t0)));
        const int end = std::max(start + 1, static_cast<int>(std::pow(static_cast<float>(half - 1), t1)) + 1);
        float peak = 0.0f;
        for (int i = start; i < end && i < half; ++i) {
            peak = std::max(peak, magnitudes[static_cast<size_t>(i)]);
        }
        levels[static_cast<size_t>(band)] = peak;
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
