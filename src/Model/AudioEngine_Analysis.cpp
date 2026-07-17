#include <Model/AudioEngine.h>

#include <DSP/DspAnalysis.h>

#include <algorithm>
#include <cmath>
#include <vector>

QVector<float> AudioEngine::waveformPoints() const
{
    return m_waveformPoints;
}

QVector<float> AudioEngine::spectrumLevels() const
{
    return m_spectrumLevels;
}

float AudioEngine::vuLevel() const
{
    return m_vuLevel;
}

float AudioEngine::peakLevel() const
{
    return m_peakLevel;
}

bool AudioEngine::clippingDetected() const
{
    return m_clippingDetected;
}

void AudioEngine::rebuildOverviewWaveform()
{
    m_waveformPoints.resize(kWaveformBins);
    if (m_tracks.isEmpty() || m_durationMs <= 0) {
        m_waveformPoints.fill(0.0f);
        return;
    }

    // Peak-hold per bin across the arrangement timeline (no per-clip wrap).
    // Content looping is audible-only; wrapping here would tile short clips and
    // make the overview look like a racing / corrupted waveform.
    for (int bin = 0; bin < kWaveformBins; ++bin) {
        const int startMs = (bin * m_durationMs) / kWaveformBins;
        const int endMs = std::max(startMs + 1, ((bin + 1) * m_durationMs) / kWaveformBins);
        float peakAbs = 0.0f;
        float signedPeak = 0.0f;
        const int step = std::max(1, (endMs - startMs) / 8);
        for (int ms = startMs; ms < endMs; ms += step) {
            const StereoSample mixed = renderMixAtMs(ms, false);
            const float mono = 0.5f * (mixed.left + mixed.right);
            const float mag = std::fabs(mono);
            if (mag >= peakAbs) {
                peakAbs = mag;
                signedPeak = mono;
            }
        }
        m_waveformPoints[bin] = std::clamp(signedPeak, -1.0f, 1.0f);
    }
}

void AudioEngine::refreshAnalysis()
{
    std::vector<float> samples(m_analysisBuffer.begin(), m_analysisBuffer.end());
    const std::vector<float> spectrum = DspAnalysis::computeSpectrumBands(samples, kSpectrumBands);

    m_spectrumLevels = QVector<float>(spectrum.begin(), spectrum.end());
    m_peakLevel = DspAnalysis::detectPeak(samples);
    m_clippingDetected = DspAnalysis::detectClipping(samples);

    // Ballistics: fast attack / slower release so loud passages fill the bar.
    const float rms = DspAnalysis::computeVuLevel(samples);
    const float target = DspAnalysis::computeDisplayMeter(m_peakLevel, rms);
    const float alpha = (target >= m_vuLevel) ? 0.62f : 0.18f;
    m_vuLevel = m_vuLevel + (target - m_vuLevel) * alpha;

    // Keep the arrangement overview waveform while playing so Seek / playhead
    // share the same timeline as the drawn clip. Live capture still drives
    // spectrum / VU / peak above.

    emit analysisChanged();
}

void AudioEngine::rebuildAnalysisBuffer()
{
    m_analysisBuffer.resize(kAnalysisBufferSamples);
    if (m_tracks.isEmpty()) {
        m_analysisBuffer.fill(0.0f);
        return;
    }

    for (int i = 0; i < kAnalysisBufferSamples; ++i) {
        const int sampleMs = m_positionMs + (i * 1000) / m_outputSampleRate;
        const StereoSample mixed = renderMixAtMs(sampleMs, false);
        m_analysisBuffer[i] = 0.5f * (mixed.left + mixed.right);
    }
}
