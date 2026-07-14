#include <Model/AudioEngine.h>

#include <QFileInfo>

#include <algorithm>

AudioEngine::AudioEngine(QObject *parent)
    : QObject(parent)
{
    connect(&m_playbackTimer, &QTimer::timeout, this, &AudioEngine::advancePlayback);
    m_playbackTimer.setInterval(kPlaybackTickMs);
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

StereoSample AudioEngine::renderMixFrame(const QVector<float> &trackMonoSamples) const
{
    StereoSample mix;
    const int count = std::min(trackMonoSamples.size(), static_cast<qsizetype>(m_tracks.size()));
    for (int i = 0; i < count; ++i) {
        const AudioTrack &track = m_tracks.at(i);
        TrackProcessParams params;
        params.volume = track.volume;
        params.pan = track.pan;
        params.audible = trackAudible(i);
        params.eqLowDb = track.eqLowDb;
        params.eqMidDb = track.eqMidDb;
        params.eqHighDb = track.eqHighDb;
        params.compThreshold = track.compThreshold;
        params.compRatio = track.compRatio;
        params.fxBypass = track.fxBypass;

        const StereoSample processed = DspProcessor::processTrackSample(trackMonoSamples.at(i), params);
        mix = DspProcessor::mixLinear(mix, processed);
    }

    return DspProcessor::applyMasterChain(mix, m_masterVolume);
}

void AudioEngine::importTrack(const QString &path)
{
    AudioTrack track;
    track.sourcePath = path;
    track.displayName = QFileInfo(path).fileName();
    if (track.displayName.isEmpty()) {
        track.displayName = QStringLiteral("Track %1").arg(m_tracks.size() + 1);
    }

    m_tracks.append(track);

    if (m_durationMs == 0) {
        setDurationMs(kPlaceholderDurationMs);
    }

    emit tracksChanged();
    emit statusMessageChanged(QStringLiteral("Imported track: %1").arg(track.displayName));
}

void AudioEngine::clearTracks()
{
    if (m_tracks.isEmpty()) {
        return;
    }

    stop();
    m_tracks.clear();
    setDurationMs(0);
    m_loopStartMs = 0;
    m_loopEndMs = 0;
    emit loopRangeChanged();
    emit tracksChanged();
    emit statusMessageChanged(QStringLiteral("All tracks cleared."));
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

void AudioEngine::setTrackVolume(int index, float volume)
{
    if (!isValidTrackIndex(index)) {
        return;
    }

    const float clamped = std::clamp(volume, 0.0f, 1.0f);
    if (qFuzzyCompare(m_tracks[index].volume, clamped)) {
        return;
    }

    m_tracks[index].volume = clamped;
    emit trackParamsChanged(index);
}

void AudioEngine::setTrackPan(int index, float pan)
{
    if (!isValidTrackIndex(index)) {
        return;
    }

    const float clamped = std::clamp(pan, -1.0f, 1.0f);
    if (qFuzzyCompare(m_tracks[index].pan, clamped)) {
        return;
    }

    m_tracks[index].pan = clamped;
    emit trackParamsChanged(index);
}

void AudioEngine::setTrackMuted(int index, bool muted)
{
    if (!isValidTrackIndex(index) || m_tracks[index].muted == muted) {
        return;
    }

    m_tracks[index].muted = muted;
    emit trackParamsChanged(index);
}

void AudioEngine::setTrackSolo(int index, bool solo)
{
    if (!isValidTrackIndex(index) || m_tracks[index].solo == solo) {
        return;
    }

    m_tracks[index].solo = solo;
    emit trackParamsChanged(index);
}

void AudioEngine::setTrackEq(int index, float lowDb, float midDb, float highDb)
{
    if (!isValidTrackIndex(index)) {
        return;
    }

    AudioTrack &track = m_tracks[index];
    const float clampedLow = std::clamp(lowDb, -12.0f, 12.0f);
    const float clampedMid = std::clamp(midDb, -12.0f, 12.0f);
    const float clampedHigh = std::clamp(highDb, -12.0f, 12.0f);
    if (qFuzzyCompare(track.eqLowDb, clampedLow) && qFuzzyCompare(track.eqMidDb, clampedMid)
        && qFuzzyCompare(track.eqHighDb, clampedHigh)) {
        return;
    }

    track.eqLowDb = clampedLow;
    track.eqMidDb = clampedMid;
    track.eqHighDb = clampedHigh;
    emit trackParamsChanged(index);
}

void AudioEngine::setTrackCompressor(int index, float threshold, float ratio)
{
    if (!isValidTrackIndex(index)) {
        return;
    }

    AudioTrack &track = m_tracks[index];
    const float clampedThreshold = std::clamp(threshold, 0.05f, 1.0f);
    const float clampedRatio = std::max(1.0f, ratio);
    if (qFuzzyCompare(track.compThreshold, clampedThreshold)
        && qFuzzyCompare(track.compRatio, clampedRatio)) {
        return;
    }

    track.compThreshold = clampedThreshold;
    track.compRatio = clampedRatio;
    emit trackParamsChanged(index);
}

void AudioEngine::setTrackFxBypass(int index, bool bypass)
{
    if (!isValidTrackIndex(index) || m_tracks[index].fxBypass == bypass) {
        return;
    }

    m_tracks[index].fxBypass = bypass;
    emit trackParamsChanged(index);
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
