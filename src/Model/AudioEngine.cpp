#include <Model/AudioEngine.h>

#include <QFileInfo>

#include <algorithm>

AudioEngine::AudioEngine(QObject *parent)
    : QObject(parent)
{
}

QStringList AudioEngine::tracks() const
{
    return m_tracks;
}

bool AudioEngine::isPlaying() const
{
    return m_isPlaying;
}

float AudioEngine::masterVolume() const
{
    return m_masterVolume;
}

void AudioEngine::importTrack(const QString &path)
{
    const QFileInfo fileInfo(path);
    const QString displayName = fileInfo.fileName().isEmpty() ? path : fileInfo.fileName();
    m_tracks.append(displayName);
    emit tracksChanged();
    emit statusMessageChanged(QStringLiteral("Imported track: %1").arg(displayName));
}

void AudioEngine::play()
{
    if (m_tracks.isEmpty()) {
        emit statusMessageChanged(QStringLiteral("Import at least one track before playback."));
        return;
    }

    m_isPlaying = true;
    emit playbackStateChanged();
    emit statusMessageChanged(QStringLiteral("Playback started."));
}

void AudioEngine::pause()
{
    m_isPlaying = false;
    emit playbackStateChanged();
    emit statusMessageChanged(QStringLiteral("Playback paused."));
}

void AudioEngine::stop()
{
    m_isPlaying = false;
    emit playbackStateChanged();
    emit statusMessageChanged(QStringLiteral("Playback stopped."));
}

void AudioEngine::setMasterVolume(float volume)
{
    m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
    emit statusMessageChanged(QStringLiteral("Master volume: %1").arg(m_masterVolume, 0, 'f', 2));
}
