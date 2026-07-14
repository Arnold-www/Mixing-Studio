#include <Model/AudioEngine.h>

#include <QFileInfo>

#include <algorithm>

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

    rebuildPlaceholderBuffer();
    refreshAnalysis();
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
    rebuildPlaceholderBuffer();
    refreshAnalysis();
    emit loopRangeChanged();
    emit tracksChanged();
    emit statusMessageChanged(QStringLiteral("All tracks cleared."));
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
