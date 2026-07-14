#include <Model/AudioEngine.h>

#include <Model/AssetLibrary.h>

#include <algorithm>

AudioEngine::AudioEngine(QObject *parent)
    : QObject(parent)
    , m_assetLibrary(new AssetLibrary(QStringLiteral("mixing_studio_assets")))
{
    connect(&m_playbackTimer, &QTimer::timeout, this, &AudioEngine::advancePlayback);
    m_playbackTimer.setInterval(kPlaybackTickMs);
    rebuildPlaceholderBuffer();
    refreshAnalysis();
}

AudioEngine::~AudioEngine()
{
    delete m_assetLibrary;
    m_assetLibrary = nullptr;
}

QStringList AudioEngine::tracks() const
{
    QStringList names;
    names.reserve(m_tracks.size());
    for (const AudioTrack &track : m_tracks) {
        names.append(track.displayName);
    }
    return names;
}

QList<AudioTrack> AudioEngine::trackData() const
{
    return m_tracks;
}

int AudioEngine::trackCount() const
{
    return m_tracks.size();
}

bool AudioEngine::isPlaying() const
{
    return m_isPlaying;
}

int AudioEngine::positionMs() const
{
    return m_positionMs;
}

int AudioEngine::durationMs() const
{
    return m_durationMs;
}

float AudioEngine::masterVolume() const
{
    return m_masterVolume;
}

int AudioEngine::loopStartMs() const
{
    return m_loopStartMs;
}

int AudioEngine::loopEndMs() const
{
    return m_loopEndMs;
}

bool AudioEngine::anySolo() const
{
    return std::any_of(m_tracks.cbegin(), m_tracks.cend(), [](const AudioTrack &track) {
        return track.solo;
    });
}

bool AudioEngine::trackAudible(int index) const
{
    if (!isValidTrackIndex(index)) {
        return false;
    }

    const AudioTrack &track = m_tracks.at(index);
    if (track.muted) {
        return false;
    }

    const bool soloActive = anySolo();
    return !soloActive || track.solo;
}

bool AudioEngine::isValidTrackIndex(int index) const
{
    return index >= 0 && index < m_tracks.size();
}

void AudioEngine::setPlaying(bool playing)
{
    if (m_isPlaying == playing) {
        return;
    }

    m_isPlaying = playing;
    emit playbackStateChanged();
}

void AudioEngine::setPositionMs(int positionMs)
{
    if (m_positionMs == positionMs) {
        return;
    }

    m_positionMs = positionMs;
    emit positionChanged();
}

void AudioEngine::setDurationMs(int durationMs)
{
    if (m_durationMs == durationMs) {
        return;
    }

    m_durationMs = durationMs;
    emit durationChanged();
}
