#include <Model/AudioEngine.h>

#include <DSP/DspAnalysis.h>

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

void AudioEngine::refreshAnalysis()
{
    std::vector<float> samples(m_analysisBuffer.begin(), m_analysisBuffer.end());
    const std::vector<float> waveform = DspAnalysis::downsampleWaveform(samples, kWaveformBins);
    const std::vector<float> spectrum = DspAnalysis::computeSpectrumBands(samples, kSpectrumBands);

    m_waveformPoints = QVector<float>(waveform.begin(), waveform.end());
    m_spectrumLevels = QVector<float>(spectrum.begin(), spectrum.end());
    m_vuLevel = DspAnalysis::computeVuLevel(samples);
    m_peakLevel = DspAnalysis::detectPeak(samples);
    m_clippingDetected = DspAnalysis::detectClipping(samples);
    emit analysisChanged();
}

void AudioEngine::rebuildPlaceholderBuffer()
{
    m_analysisBuffer.resize(kAnalysisBufferSamples);
    const float phaseOffset = static_cast<float>(m_positionMs) * 0.001f;
    const float amplitude = m_tracks.isEmpty() ? 0.0f : 0.55f * m_masterVolume;
    for (int i = 0; i < kAnalysisBufferSamples; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(kAnalysisBufferSamples);
        const float sample = amplitude * std::sin(2.0f * 3.14159265f * (4.0f + static_cast<float>(m_tracks.size())) * t
                                                  + phaseOffset);
        m_analysisBuffer[i] = sample;
    }
}
