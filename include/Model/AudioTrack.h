#pragma once

#include <QString>
#include <QVector>

#include <algorithm>
#include <cmath>

struct AutomationPoint
{
    int timeMs = 0;
    float value = 1.0f; // relative volume multiplier 0..1
};

struct AudioTrack
{
    static constexpr int kEqBandCount = 10;

    QString sourcePath;
    QString displayName;
    QVector<float> pcmMono;
    int sampleRate = 44100;
    float volume = 0.8f;
    float pan = 0.0f;
    float playbackRate = 1.0f; // 0.5 .. 2.0 typical; stretches/compresses time
    bool muted = false;
    bool solo = false;
    float eqLowDb = 0.0f;
    float eqMidDb = 0.0f;
    float eqHighDb = 0.0f;
    QVector<float> eqBands; // 10-band graphic EQ gains in dB
    float compThreshold = 0.7f;
    float compRatio = 4.0f;
    bool fxBypass = false;
    bool loopEnabled = false;          // per-track content loop
    float loopStart = 0.0f;            // 0..1 of clip
    float loopEnd = 1.0f;              // 0..1 of clip
    QVector<AutomationPoint> volumeAutomation;

    void ensureEqBands()
    {
        if (eqBands.size() == kEqBandCount) {
            return;
        }
        eqBands = defaultEqBands();
    }

    QVector<float> defaultEqBands() const
    {
        QVector<float> bands(kEqBandCount, 0.0f);
        bands[1] = eqLowDb;
        bands[4] = eqMidDb;
        bands[7] = eqHighDb;
        return bands;
    }

    QVector<float> eqBandsOrDefault() const
    {
        if (eqBands.size() == kEqBandCount) {
            return eqBands;
        }
        return defaultEqBands();
    }

    void syncLegacyEqFromBands()
    {
        ensureEqBands();
        eqLowDb = eqBands[1];
        eqMidDb = eqBands[4];
        eqHighDb = eqBands[7];
    }

    void syncBandsFromLegacyEq()
    {
        ensureEqBands();
        eqBands[1] = eqLowDb;
        eqBands[4] = eqMidDb;
        eqBands[7] = eqHighDb;
    }

    int durationMs() const
    {
        if (pcmMono.isEmpty() || sampleRate <= 0) {
            return 0;
        }
        const float rate = std::clamp(playbackRate, 0.25f, 4.0f);
        return static_cast<int>((static_cast<qint64>(pcmMono.size()) * 1000)
                                / (static_cast<double>(sampleRate) * static_cast<double>(rate)));
    }

    // Wall-clock length of one content cycle (accounts for playbackRate).
    double contentDurationSeconds() const
    {
        if (pcmMono.isEmpty() || sampleRate <= 0) {
            return 0.0;
        }
        const float rate = std::clamp(playbackRate, 0.25f, 4.0f);
        return static_cast<double>(pcmMono.size())
               / (static_cast<double>(sampleRate) * static_cast<double>(rate));
    }

    // Sample-accurate read. Prefer this for playback/export — integer ms alone
    // holds each source sample for ~44 output frames at 44.1 kHz (~1 kHz quality).
    // Loops when loopEnabled (or loopContent override) within [loopStart, loopEnd].
    float sampleAtSeconds(double seconds, bool loopContent = false) const
    {
        if (pcmMono.isEmpty() || sampleRate <= 0) {
            return 0.0f;
        }
        const float rate = std::clamp(playbackRate, 0.25f, 4.0f);
        const double length = static_cast<double>(pcmMono.size());
        double index = seconds * static_cast<double>(sampleRate) * static_cast<double>(rate);
        const bool loop = loopContent || loopEnabled;
        if (loop) {
            const double start = std::clamp(static_cast<double>(loopStart), 0.0, 1.0) * length;
            const double end = std::max(start + 1.0,
                                        std::clamp(static_cast<double>(loopEnd), 0.0, 1.0) * length);
            const double region = end - start;
            index = std::fmod(index, region);
            if (index < 0.0) {
                index += region;
            }
            index += start;
            if (index >= length) {
                index = start;
            }
        } else if (index < 0.0 || index >= length) {
            return 0.0f;
        }
        const int i0 = static_cast<int>(index);
        const int last = static_cast<int>(pcmMono.size()) - 1;
        const int i1 = i0 < last ? i0 + 1 : last;
        const float frac = static_cast<float>(index - static_cast<double>(i0));
        return pcmMono.at(i0) * (1.0f - frac) + pcmMono.at(i1) * frac;
    }

    float sampleAtMs(int positionMs, bool loopContent = false) const
    {
        return sampleAtSeconds(static_cast<double>(positionMs) / 1000.0, loopContent);
    }

    float automationVolumeAtSeconds(double seconds) const
    {
        if (volumeAutomation.isEmpty()) {
            return 1.0f;
        }
        if (volumeAutomation.size() == 1) {
            return std::clamp(volumeAutomation.first().value, 0.0f, 1.0f);
        }

        const double positionMs = seconds * 1000.0;
        if (positionMs <= volumeAutomation.first().timeMs) {
            return std::clamp(volumeAutomation.first().value, 0.0f, 1.0f);
        }
        if (positionMs >= volumeAutomation.last().timeMs) {
            return std::clamp(volumeAutomation.last().value, 0.0f, 1.0f);
        }

        for (int i = 0; i + 1 < volumeAutomation.size(); ++i) {
            const AutomationPoint &a = volumeAutomation.at(i);
            const AutomationPoint &b = volumeAutomation.at(i + 1);
            if (positionMs >= a.timeMs && positionMs <= b.timeMs) {
                const double span = std::max(1.0, static_cast<double>(b.timeMs - a.timeMs));
                const float t = static_cast<float>((positionMs - a.timeMs) / span);
                return std::clamp(a.value + (b.value - a.value) * t, 0.0f, 1.0f);
            }
        }
        return 1.0f;
    }

    float automationVolumeAtMs(int positionMs) const
    {
        return automationVolumeAtSeconds(static_cast<double>(positionMs) / 1000.0);
    }
};
