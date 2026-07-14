#include <Model/AudioEngine.h>

#include <algorithm>

void AudioEngine::play()
{
    if (m_tracks.isEmpty()) {
        emit statusMessageChanged(QStringLiteral("Import at least one track before playback."));
        return;
    }

    if (m_positionMs >= m_durationMs && m_durationMs > 0) {
        setPositionMs(0);
    }

    setPlaying(true);
    m_playbackTimer.start();
    emit statusMessageChanged(QStringLiteral("Playback started."));
}

void AudioEngine::pause()
{
    if (!m_isPlaying) {
        return;
    }

    setPlaying(false);
    m_playbackTimer.stop();
    emit statusMessageChanged(QStringLiteral("Playback paused."));
}

void AudioEngine::stop()
{
    setPlaying(false);
    m_playbackTimer.stop();
    setPositionMs(0);
    emit statusMessageChanged(QStringLiteral("Playback stopped."));
}

void AudioEngine::seek(int positionMs)
{
    const int clamped = std::clamp(positionMs, 0, m_durationMs);
    if (m_positionMs == clamped) {
        return;
    }

    setPositionMs(clamped);
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
    const bool hasLoop = m_loopEndMs > m_loopStartMs;

    if (nextPosition >= m_durationMs) {
        if (hasLoop) {
            setPositionMs(m_loopStartMs);
        } else {
            setPositionMs(m_durationMs);
            pause();
        }
    } else if (hasLoop && nextPosition >= m_loopEndMs) {
        setPositionMs(m_loopStartMs);
    } else {
        setPositionMs(nextPosition);
    }

    rebuildPlaceholderBuffer();
    refreshAnalysis();
}
