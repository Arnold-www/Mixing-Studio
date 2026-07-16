#include <Model/AudioEngine.h>

#include <Model/AssetLibrary.h>

#include <algorithm>
#include <cmath>

MixAudioDevice::MixAudioDevice(AudioEngine *engine, QObject *parent)
    : QIODevice(parent)
    , m_engine(engine)
{
}

bool MixAudioDevice::isSequential() const
{
    return true;
}

qint64 MixAudioDevice::bytesAvailable() const
{
    // Pull-mode sink: always report data ready while open.
    return static_cast<qint64>(1) << 20;
}

qint64 MixAudioDevice::size() const
{
    return bytesAvailable();
}

qint64 MixAudioDevice::readData(char *data, qint64 maxlen)
{
    if (!m_engine || !data || maxlen <= 0) {
        return 0;
    }
    return m_engine->pullInterleavedPcm(data, static_cast<int>(maxlen));
}

qint64 MixAudioDevice::writeData(const char *, qint64)
{
    return -1;
}

AudioEngine::AudioEngine(QObject *parent)
    : QObject(parent)
    , m_assetLibrary(new AssetLibrary(QStringLiteral("mixing_studio_assets")))
{
    connect(&m_playbackTimer, &QTimer::timeout, this, &AudioEngine::advancePlayback);
    m_playbackTimer.setInterval(kPlaybackTickMs);
    connect(&m_meterTimer, &QTimer::timeout, this, &AudioEngine::onMeterTimer);
    m_meterTimer.setInterval(kMeterTickMs);
    m_meterTimer.setTimerType(Qt::PreciseTimer);
    m_captureRing.resize(kAnalysisBufferSamples);
    m_captureRing.fill(0.0f);
    rebuildAnalysisBuffer();
    rebuildOverviewWaveform();
    refreshAnalysis();
}

AudioEngine::~AudioEngine()
{
    stopAudioOutput();
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
    // UI / ViewModel read the audible clock. Internal write-head stays in m_positionMs.
    return audiblePositionMs();
}

qint64 AudioEngine::positionUSecs() const
{
    return audiblePositionUSecs();
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

int AudioEngine::outputSampleRate() const
{
    return m_outputSampleRate;
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
        m_playheadSamples = (static_cast<qint64>(m_positionMs) * m_outputSampleRate) / 1000;
        return;
    }

    m_positionMs = positionMs;
    m_playheadSamples = (static_cast<qint64>(m_positionMs) * m_outputSampleRate) / 1000;
    m_lastQueuedPositionMs = m_positionMs;
    m_lastEmittedPositionUSecs = static_cast<qint64>(m_positionMs) * 1000;
    m_lastDeviceProcessedUSecs = -1;
    m_smoothAudibleClock.invalidate();
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

void AudioEngine::recomputeDuration()
{
    int maxDuration = 0;
    for (const AudioTrack &track : m_tracks) {
        maxDuration = std::max(maxDuration, track.durationMs());
    }
    setDurationMs(maxDuration);
    if (m_loopEndMs > m_durationMs) {
        m_loopEndMs = m_durationMs;
        emit loopRangeChanged();
    }
}

AudioTrack AudioEngine::makePlaceholderTrack(const QString &name) const
{
    AudioTrack track;
    track.sourcePath = name;
    track.displayName = name;
    track.sampleRate = m_outputSampleRate;
    const int frames = (kPlaceholderDurationMs * m_outputSampleRate) / 1000;
    track.pcmMono.resize(frames);
    for (int i = 0; i < frames; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(m_outputSampleRate);
        track.pcmMono[i] = 0.25f * std::sin(2.0f * 3.14159265f * 440.0f * t);
    }
    return track;
}
