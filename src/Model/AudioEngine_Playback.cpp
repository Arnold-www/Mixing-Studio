#include <Model/AudioEngine.h>

#include <QMediaDevices>
#include <QMetaObject>
#include <QMutexLocker>

#include <algorithm>
#include <cmath>

QAudioFormat AudioEngine::chooseOutputFormat(const QAudioDevice &device) const
{
    const QAudioFormat preferred = device.preferredFormat();
    const int preferredRate = preferred.sampleRate() > 0 ? preferred.sampleRate() : m_outputSampleRate;
    const int preferredChannels = preferred.channelCount() > 0 ? preferred.channelCount() : 2;

    // pullInterleavedPcm intentionally supports only these two formats. Never
    // advertise a preferred Int32/UInt8 format and then write Int16 bytes into it.
    const int rates[] = {m_outputSampleRate, preferredRate};
    const int channels[] = {2, preferredChannels, 1};
    const QAudioFormat::SampleFormat formats[] = {QAudioFormat::Int16, QAudioFormat::Float};
    for (QAudioFormat::SampleFormat sampleFormat : formats) {
        for (int rate : rates) {
            for (int channelCount : channels) {
                QAudioFormat candidate;
                candidate.setSampleRate(rate);
                candidate.setChannelCount(channelCount);
                candidate.setSampleFormat(sampleFormat);
                if (device.isFormatSupported(candidate)) {
                    return candidate;
                }
            }
        }
    }

    // QAudioSink will report an error if even this conservative format cannot
    // be opened, at which point playback falls back to the timer clock.
    QAudioFormat fallback;
    fallback.setSampleRate(preferredRate);
    fallback.setChannelCount(2);
    fallback.setSampleFormat(QAudioFormat::Int16);
    return fallback;
}

void AudioEngine::ensureAudioOutput()
{
    if (m_audioSink) {
        return;
    }

    const QAudioDevice device = QMediaDevices::defaultAudioOutput();
    if (device.isNull()) {
        emit statusMessageChanged(QStringLiteral("No default audio output device; using timer clock."));
        m_useAudioSink = false;
        return;
    }

    const QAudioFormat format = chooseOutputFormat(device);
    m_outputSampleRate = format.sampleRate() > 0 ? format.sampleRate() : kExportSampleRate;
    m_outputChannelCount = format.channelCount() > 0 ? format.channelCount() : 2;
    m_outputSampleFormat = format.sampleFormat();
    m_playheadSamples = (static_cast<qint64>(m_positionMs) * m_outputSampleRate) / 1000;

    m_audioDevice = new MixAudioDevice(this, this);
    m_audioDevice->open(QIODevice::ReadOnly);

    m_audioSink = new QAudioSink(device, format, this);
    // ~120ms buffer: low enough for Seek/playhead to track audio, high enough for UI load.
    const int bytesPerFrame = m_outputChannelCount * (m_outputSampleFormat == QAudioFormat::Float ? 4 : 2);
    m_audioSink->setBufferSize(m_outputSampleRate * bytesPerFrame * 12 / 100);
    connect(m_audioSink, &QAudioSink::stateChanged, this, &AudioEngine::onAudioStateChanged);
    m_useAudioSink = true;

    emit statusMessageChanged(
        QStringLiteral("Audio device: %1 (%2 Hz, %3 ch, %4)")
            .arg(device.description())
            .arg(m_outputSampleRate)
            .arg(m_outputChannelCount)
            .arg(m_outputSampleFormat == QAudioFormat::Float ? QStringLiteral("Float")
                                                             : QStringLiteral("Int16")));
}

void AudioEngine::stopAudioOutput()
{
    if (m_audioSink) {
        m_audioSink->stop();
    }
}

void AudioEngine::startUiMeterTimer()
{
    if (!m_meterTimer.isActive()) {
        m_meterTimer.start();
    }
}

void AudioEngine::stopUiMeterTimer()
{
    m_meterTimer.stop();
}

void AudioEngine::onAudioStateChanged(QAudio::State state)
{
    if (state == QAudio::IdleState && m_isPlaying && m_durationMs > 0 && m_positionMs >= m_durationMs) {
        pause();
        return;
    }

    if (state == QAudio::StoppedState && m_audioSink && m_audioSink->error() != QAudio::NoError) {
        emit statusMessageChanged(
            QStringLiteral("Audio sink error %1; falling back to timer clock.").arg(static_cast<int>(m_audioSink->error())));
        m_useAudioSink = false;
        if (m_isPlaying && !m_playbackTimer.isActive()) {
            m_playbackTimer.start();
            startUiMeterTimer();
        }
    }
}

void AudioEngine::schedulePositionNotify()
{
    // Audible UI clock is driven from onMeterTimer via processedUSecs().
    // Keep this as a lightweight fallback when the sink is not used.
    if (m_useAudioSink && m_audioSink) {
        return;
    }
    if (m_positionNotifyPending) {
        return;
    }
    if (m_lastQueuedPositionMs >= 0
        && std::abs(m_positionMs - m_lastQueuedPositionMs) < kPositionUiMinDeltaMs) {
        return;
    }
    m_lastQueuedPositionMs = m_positionMs;
    m_positionNotifyPending = true;
    QMetaObject::invokeMethod(this, &AudioEngine::emitQueuedPositionChanged, Qt::QueuedConnection);
}

void AudioEngine::emitQueuedPositionChanged()
{
    m_positionNotifyPending = false;
    const qint64 audibleUs = audiblePositionUSecs();
    if (audibleUs != m_lastEmittedPositionUSecs) {
        m_lastEmittedPositionUSecs = audibleUs;
        emit positionChanged();
    }
}

qint64 AudioEngine::wrapAudibleUSecs(qint64 heardUs) const
{
    if (m_durationMs <= 0) {
        return std::max<qint64>(0, heardUs);
    }

    const qint64 durationUs = static_cast<qint64>(m_durationMs) * 1000;

    // Transport (song) loop only — per-track Loop never wraps the playhead clock.
    if (m_loopEndMs > m_loopStartMs) {
        const qint64 loopStartUs = static_cast<qint64>(m_loopStartMs) * 1000;
        const qint64 loopLenUs = static_cast<qint64>(m_loopEndMs - m_loopStartMs) * 1000;
        if (loopLenUs > 0 && heardUs >= loopStartUs) {
            return loopStartUs + ((heardUs - loopStartUs) % loopLenUs);
        }
    }

    return std::clamp(heardUs, qint64{0}, durationUs);
}

void AudioEngine::resyncSmoothAudibleClock()
{
    m_lastDeviceProcessedUSecs = -1;
    m_smoothAudibleClock.invalidate();
}

qint64 AudioEngine::audiblePositionUSecs() const
{
    if (!m_isPlaying || !m_useAudioSink || !m_audioSink || m_durationMs <= 0) {
        return static_cast<qint64>(m_positionMs) * 1000;
    }

    // Prefer last sampled device time (updated on meter ticks). Extrapolate with
    // wall clock so Seek moves every frame even when WASAPI jumps in chunks.
    const qint64 processed = (m_lastDeviceProcessedUSecs >= 0)
        ? m_lastDeviceProcessedUSecs
        : std::max<qint64>(0, m_audioSink->processedUSecs());

    qint64 deviceUs = static_cast<qint64>(m_playStartPositionMs) * 1000 + processed;
    if (m_smoothAudibleClock.isValid()) {
        deviceUs += std::min<qint64>(m_smoothAudibleClock.nsecsElapsed() / 1000,
                                    kSmoothExtrapolateMaxUs);
    }

    return wrapAudibleUSecs(deviceUs);
}

int AudioEngine::audiblePositionMs() const
{
    return static_cast<int>(audiblePositionUSecs() / 1000);
}

void AudioEngine::syncTransportFromAudible()
{
    const int audible = audiblePositionMs();
    m_positionMs = audible;
    m_playheadSamples = (static_cast<qint64>(m_positionMs) * m_outputSampleRate) / 1000;
    m_lastQueuedPositionMs = m_positionMs;
    m_lastEmittedPositionUSecs = static_cast<qint64>(m_positionMs) * 1000;
    resyncSmoothAudibleClock();
}

void AudioEngine::captureAnalysisSample(float mono)
{
    QMutexLocker lock(&m_captureMutex);
    if (m_captureRing.isEmpty()) {
        m_captureRing.resize(kAnalysisBufferSamples);
        m_captureRing.fill(0.0f);
        m_captureWrite = 0;
    }
    if ((++m_captureDecimateCounter % kCaptureDecimate) != 0) {
        return;
    }
    m_captureRing[m_captureWrite] = mono;
    m_captureWrite = (m_captureWrite + 1) % m_captureRing.size();
}

void AudioEngine::onMeterTimer()
{
    if (!m_isPlaying) {
        return;
    }

    // Sample device clock, then extrapolate between WASAPI period jumps.
    if (m_useAudioSink && m_audioSink) {
        const qint64 processed = std::max<qint64>(0, m_audioSink->processedUSecs());
        if (processed != m_lastDeviceProcessedUSecs) {
            m_lastDeviceProcessedUSecs = processed;
            m_smoothAudibleClock.restart();
        } else if (!m_smoothAudibleClock.isValid()) {
            m_lastDeviceProcessedUSecs = processed;
            m_smoothAudibleClock.start();
        }
    }

    ++m_meterFrame;
    if ((m_meterFrame % kAnalysisEveryNMeters) == 0) {
        bool copiedCapture = false;
        {
            QMutexLocker lock(&m_captureMutex);
            if (!m_captureRing.isEmpty()) {
                m_analysisBuffer.resize(m_captureRing.size());
                const int size = m_captureRing.size();
                for (int i = 0; i < size; ++i) {
                    m_analysisBuffer[i] = m_captureRing[(m_captureWrite + i) % size];
                }
                copiedCapture = true;
            }
        }
        if (!copiedCapture) {
            rebuildAnalysisBuffer();
        }
        refreshAnalysis();
    }

    // ~120 Hz Seek / playhead (sub-ms progress via positionUSecs).
    const qint64 audibleUs = audiblePositionUSecs();
    if (audibleUs != m_lastEmittedPositionUSecs) {
        m_lastEmittedPositionUSecs = audibleUs;
        emit positionChanged();
    }
}

StereoSample AudioEngine::renderMixAtPlayhead() const
{
    const double seconds = static_cast<double>(m_playheadSamples)
                           / static_cast<double>(std::max(1, m_outputSampleRate));
    // Per-track loopEnabled only (force flag unused by trackSamplesAtSeconds).
    return renderMixAtSeconds(seconds, false);
}

int AudioEngine::pullInterleavedInt16(char *data, int maxBytes)
{
    return pullInterleavedPcm(data, maxBytes);
}

int AudioEngine::pullInterleavedPcm(char *data, int maxBytes)
{
    if (!data || maxBytes <= 0 || m_tracks.isEmpty() || !m_isPlaying) {
        return 0;
    }

    const int bytesPerSample = (m_outputSampleFormat == QAudioFormat::Float) ? 4 : 2;
    const int bytesPerFrame = m_outputChannelCount * bytesPerSample;
    if (bytesPerFrame <= 0 || maxBytes < bytesPerFrame) {
        return 0;
    }

    const int framesRequested = maxBytes / bytesPerFrame;
    int framesWritten = 0;

    const qint64 durationSamples = m_durationMs > 0
        ? (static_cast<qint64>(m_durationMs) * m_outputSampleRate) / 1000
        : 0;

    const bool transportLoop = m_loopEndMs > m_loopStartMs;
    const qint64 loopStartSamples = transportLoop
        ? (static_cast<qint64>(m_loopStartMs) * m_outputSampleRate) / 1000
        : 0;
    const qint64 loopEndSamples = transportLoop
        ? (static_cast<qint64>(m_loopEndMs) * m_outputSampleRate) / 1000
        : durationSamples;

    for (int i = 0; i < framesRequested; ++i) {
        if (transportLoop && loopEndSamples > loopStartSamples
            && m_playheadSamples >= loopEndSamples) {
            m_playheadSamples = loopStartSamples;
        } else if (!transportLoop && durationSamples > 0
                   && m_playheadSamples >= durationSamples) {
            break;
        }

        const StereoSample mixed = renderMixAtPlayhead();
        const float left = std::clamp(mixed.left, -1.0f, 1.0f);
        const float right = std::clamp(mixed.right, -1.0f, 1.0f);
        captureAnalysisSample(0.5f * (left + right));

        char *framePtr = data + (framesWritten * bytesPerFrame);
        if (m_outputSampleFormat == QAudioFormat::Float) {
            auto *out = reinterpret_cast<float *>(framePtr);
            out[0] = left;
            if (m_outputChannelCount > 1) {
                out[1] = right;
            }
            for (int ch = 2; ch < m_outputChannelCount; ++ch) {
                out[ch] = 0.0f;
            }
        } else {
            auto *out = reinterpret_cast<qint16 *>(framePtr);
            out[0] = static_cast<qint16>(std::lround(left * 32767.0f));
            if (m_outputChannelCount > 1) {
                out[1] = static_cast<qint16>(std::lround(right * 32767.0f));
            }
            for (int ch = 2; ch < m_outputChannelCount; ++ch) {
                out[ch] = 0;
            }
        }

        ++framesWritten;
        ++m_playheadSamples;
    }

    if (framesWritten > 0) {
        const int newMs = static_cast<int>((m_playheadSamples * 1000) / m_outputSampleRate);
        if (newMs != m_positionMs) {
            m_positionMs = newMs;
            schedulePositionNotify();
        }
    }

    return framesWritten * bytesPerFrame;
}

void AudioEngine::play()
{
    if (m_tracks.isEmpty()) {
        emit statusMessageChanged(QStringLiteral("Import at least one track before playback."));
        return;
    }

    if (m_positionMs >= m_durationMs && m_durationMs > 0) {
        setPositionMs(0);
    }

    const bool forceTimerClock = qEnvironmentVariableIntValue("MIXING_STUDIO_FORCE_TIMER_CLOCK") != 0;
    if (!forceTimerClock) {
        ensureAudioOutput();
    } else {
        m_useAudioSink = false;
    }

    setPlaying(true);
    startUiMeterTimer();

    if (m_useAudioSink && m_audioSink && m_audioDevice) {
        m_playbackTimer.stop();
        m_playStartPositionMs = m_positionMs;
        if (m_audioSink->state() != QAudio::ActiveState) {
            m_audioSink->start(m_audioDevice);
        } else {
            // Ensure processedUSecs baseline matches current timeline origin.
            m_audioSink->stop();
            m_audioSink->start(m_audioDevice);
        }
        if (m_audioSink->error() != QAudio::NoError) {
            emit statusMessageChanged(
                QStringLiteral("Failed to start audio sink (error %1); using timer clock.")
                    .arg(static_cast<int>(m_audioSink->error())));
            m_useAudioSink = false;
            m_playbackTimer.start();
            return;
        }
        resyncSmoothAudibleClock();
        m_lastEmittedPositionUSecs = static_cast<qint64>(m_playStartPositionMs) * 1000;
        emit positionChanged();
        emit statusMessageChanged(QStringLiteral("Playback started (audio device)."));
        return;
    }

    m_playbackTimer.start();
    emit statusMessageChanged(QStringLiteral("Playback started (timer clock)."));
}

void AudioEngine::pause()
{
    if (!m_isPlaying) {
        return;
    }

    // Freeze transport at what the user actually heard, not the buffer write-head.
    if (m_useAudioSink && m_audioSink) {
        syncTransportFromAudible();
    }

    setPlaying(false);
    m_playbackTimer.stop();
    stopUiMeterTimer();
    stopAudioOutput();
    m_lastQueuedPositionMs = -1;
    rebuildOverviewWaveform();
    emit analysisChanged();
    emit positionChanged();
    emit statusMessageChanged(QStringLiteral("Playback paused."));
}

void AudioEngine::stop()
{
    setPlaying(false);
    m_playbackTimer.stop();
    stopUiMeterTimer();
    stopAudioOutput();
    setPositionMs(0);
    rebuildAnalysisBuffer();
    rebuildOverviewWaveform();
    refreshAnalysis();
    emit statusMessageChanged(QStringLiteral("Playback stopped."));
}

void AudioEngine::seek(int positionMs)
{
    const int clamped = std::clamp(positionMs, 0, m_durationMs);
    const bool wasPlaying = m_isPlaying;
    if (m_positionMs == clamped && !wasPlaying) {
        return;
    }

    if (wasPlaying && m_useAudioSink && m_audioSink) {
        // Restart sink so processedUSecs stays aligned with the new origin.
        m_audioSink->stop();
        setPositionMs(clamped);
        m_playStartPositionMs = clamped;
        resyncSmoothAudibleClock();
        m_audioSink->start(m_audioDevice);
    } else {
        setPositionMs(clamped);
        m_playStartPositionMs = clamped;
        resyncSmoothAudibleClock();
    }

    rebuildAnalysisBuffer();
    refreshAnalysis();
    emit statusMessageChanged(QStringLiteral("Seek to %1 ms.").arg(clamped));
}

void AudioEngine::setMasterVolume(float volume)
{
    const float clamped = std::clamp(volume, 0.0f, 1.0f);
    if (qFuzzyCompare(m_masterVolume, clamped)) {
        return;
    }

    m_masterVolume = clamped;
    emit masterVolumeChanged();
    // Avoid heavy analysis rebuild while dragging the master slider.
    emit statusMessageChanged(QStringLiteral("Master volume: %1").arg(m_masterVolume, 0, 'f', 2));
}

void AudioEngine::setLoopRange(int startMs, int endMs)
{
    const int clampedStart = std::clamp(startMs, 0, m_durationMs);
    const int clampedEnd = std::clamp(endMs, clampedStart, m_durationMs);
    if (m_loopStartMs == clampedStart && m_loopEndMs == clampedEnd) {
        return;
    }

    m_loopStartMs = clampedStart;
    m_loopEndMs = clampedEnd;
    emit loopRangeChanged();
    emit statusMessageChanged(
        QStringLiteral("Loop range: %1-%2 ms.").arg(m_loopStartMs).arg(m_loopEndMs));
}

void AudioEngine::advancePlayback()
{
    if (!m_isPlaying) {
        return;
    }

    const int nextPosition = m_positionMs + kPlaybackTickMs;
    const bool transportLoop = m_loopEndMs > m_loopStartMs;

    if (transportLoop && nextPosition >= m_loopEndMs) {
        setPositionMs(m_loopStartMs);
    } else if (!transportLoop && nextPosition >= m_durationMs) {
        setPositionMs(m_durationMs);
        pause();
    } else {
        setPositionMs(nextPosition);
    }

    rebuildAnalysisBuffer();
    refreshAnalysis();
}
