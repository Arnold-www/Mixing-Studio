#include <Model/AudioEngine.h>

#include <Model/AudioFileDecoder.h>

#include <QFileInfo>

#include <algorithm>
#include <cmath>

void AudioEngine::importTrack(const QString &path)
{
    if (!importAudioFile(path)) {
        // Keep mock/demo import working when path is not a real audio file.
        AudioTrack track = makePlaceholderTrack(QFileInfo(path).fileName().isEmpty()
                                                    ? path
                                                    : QFileInfo(path).fileName());
        if (track.displayName.isEmpty()) {
            track.displayName = QStringLiteral("Track %1").arg(m_tracks.size() + 1);
        }
        track.sourcePath = path;
        m_tracks.append(track);
        recomputeDuration();
        rebuildAnalysisBuffer();
        rebuildOverviewWaveform();
        refreshAnalysis();
        emit tracksChanged();
        emit statusMessageChanged(QStringLiteral("Imported placeholder track: %1").arg(track.displayName));
    }
}

bool AudioEngine::importWavFile(const QString &path)
{
    return importAudioFile(path);
}

bool AudioEngine::importAudioFile(const QString &path)
{
    WavDecodeResult decoded;
    QString error;
    if (!AudioFileDecoder::decodeFile(path, &decoded, &error)) {
        emit statusMessageChanged(error);
        return false;
    }

    AudioTrack track;
    track.sourcePath = path;
    track.displayName = QFileInfo(path).fileName();
    track.pcmMono = decoded.monoSamples;
    track.sampleRate = decoded.sampleRate > 0 ? decoded.sampleRate : m_outputSampleRate;
    m_tracks.append(track);

    recomputeDuration();
    rebuildAnalysisBuffer();
    rebuildOverviewWaveform();
    refreshAnalysis();
    emit tracksChanged();
    emit statusMessageChanged(QStringLiteral("Imported audio: %1 (%2 ms)")
                                  .arg(track.displayName)
                                  .arg(track.durationMs()));
    return true;
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
    rebuildAnalysisBuffer();
    rebuildOverviewWaveform();
    refreshAnalysis();
    emit loopRangeChanged();
    emit tracksChanged();
    emit statusMessageChanged(QStringLiteral("All tracks cleared."));
}

bool AudioEngine::removeTrack(int index)
{
    if (!isValidTrackIndex(index)) {
        return false;
    }

    if (m_isPlaying) {
        pause();
    }

    const QString name = m_tracks.at(index).displayName;
    m_tracks.removeAt(index);
    if (m_tracks.isEmpty()) {
        setDurationMs(0);
        m_loopStartMs = 0;
        m_loopEndMs = 0;
        setPositionMs(0);
        emit loopRangeChanged();
    } else {
        recomputeDuration();
        if (m_positionMs > m_durationMs) {
            setPositionMs(m_durationMs);
        }
    }

    rebuildAnalysisBuffer();
    rebuildOverviewWaveform();
    refreshAnalysis();
    emit tracksChanged();
    emit statusMessageChanged(QStringLiteral("Removed track: %1").arg(name));
    return true;
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

void AudioEngine::setTrackPlaybackRate(int index, float rate)
{
    if (!isValidTrackIndex(index)) {
        return;
    }

    const float clamped = std::clamp(rate, 0.5f, 2.0f);
    if (qFuzzyCompare(m_tracks[index].playbackRate, clamped)) {
        return;
    }

    m_tracks[index].playbackRate = clamped;
    recomputeDuration();
    rebuildOverviewWaveform();
    emit trackParamsChanged(index);
    emit analysisChanged();
    emit statusMessageChanged(QStringLiteral("Track %1 speed: %2x")
                                  .arg(index + 1)
                                  .arg(clamped, 0, 'f', 2));
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

    if (qFuzzyCompare(m_tracks[index].eqLowDb, lowDb) && qFuzzyCompare(m_tracks[index].eqMidDb, midDb)
        && qFuzzyCompare(m_tracks[index].eqHighDb, highDb)) {
        return;
    }

    m_tracks[index].eqLowDb = lowDb;
    m_tracks[index].eqMidDb = midDb;
    m_tracks[index].eqHighDb = highDb;
    m_tracks[index].syncBandsFromLegacyEq();
    emit trackParamsChanged(index);
}

void AudioEngine::setTrackEqBands(int index, const QVector<float> &bands)
{
    if (!isValidTrackIndex(index)) {
        return;
    }

    QVector<float> normalized(AudioTrack::kEqBandCount, 0.0f);
    for (int i = 0; i < AudioTrack::kEqBandCount; ++i) {
        normalized[i] = i < bands.size() ? std::clamp(bands.at(i), -12.0f, 12.0f) : 0.0f;
    }
    m_tracks[index].ensureEqBands();
    if (m_tracks[index].eqBands == normalized) {
        return;
    }

    m_tracks[index].eqBands = normalized;
    m_tracks[index].syncLegacyEqFromBands();
    emit trackParamsChanged(index);
}

void AudioEngine::setTrackEqBand(int index, int band, float gainDb)
{
    if (!isValidTrackIndex(index) || band < 0 || band >= AudioTrack::kEqBandCount) {
        return;
    }

    m_tracks[index].ensureEqBands();
    const float clamped = std::clamp(gainDb, -12.0f, 12.0f);
    if (qFuzzyCompare(m_tracks[index].eqBands[band], clamped)) {
        return;
    }

    m_tracks[index].eqBands[band] = clamped;
    m_tracks[index].syncLegacyEqFromBands();
    emit trackParamsChanged(index);
}

void AudioEngine::setTrackCompressor(int index, float threshold, float ratio)
{
    if (!isValidTrackIndex(index)) {
        return;
    }

    if (qFuzzyCompare(m_tracks[index].compThreshold, threshold)
        && qFuzzyCompare(m_tracks[index].compRatio, ratio)) {
        return;
    }

    m_tracks[index].compThreshold = threshold;
    m_tracks[index].compRatio = ratio;
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

void AudioEngine::setTrackLoopEnabled(int index, bool enabled)
{
    if (!isValidTrackIndex(index) || m_tracks[index].loopEnabled == enabled) {
        return;
    }

    m_tracks[index].loopEnabled = enabled;
    emit trackParamsChanged(index);
}

void AudioEngine::setTrackLoopRange(int index, float startProgress, float endProgress)
{
    if (!isValidTrackIndex(index)) {
        return;
    }

    float s = std::clamp(startProgress, 0.0f, 1.0f);
    float e = std::clamp(endProgress, 0.0f, 1.0f);
    if (e < s + 0.02f) {
        e = std::min(1.0f, s + 0.02f);
    }
    if (qFuzzyCompare(m_tracks[index].loopStart, s) && qFuzzyCompare(m_tracks[index].loopEnd, e)) {
        return;
    }

    m_tracks[index].loopStart = s;
    m_tracks[index].loopEnd = e;
    emit trackParamsChanged(index);
}

void AudioEngine::setTrackVolumeAutomation(int index, const QVector<AutomationPoint> &points)
{
    if (!isValidTrackIndex(index)) {
        return;
    }

    QVector<AutomationPoint> cleaned;
    cleaned.reserve(points.size());
    for (const AutomationPoint &point : points) {
        AutomationPoint item;
        item.timeMs = std::max(0, point.timeMs);
        item.value = std::clamp(point.value, 0.0f, 1.0f);
        cleaned.append(item);
    }
    std::sort(cleaned.begin(), cleaned.end(), [](const AutomationPoint &a, const AutomationPoint &b) {
        return a.timeMs < b.timeMs;
    });

    m_tracks[index].volumeAutomation = cleaned;
    rebuildOverviewWaveform();
    emit trackParamsChanged(index);
    emit analysisChanged();
}

float AudioEngine::trackAutomationVolumeAtMs(int index, int positionMs) const
{
    if (!isValidTrackIndex(index)) {
        return 1.0f;
    }
    return m_tracks.at(index).automationVolumeAtMs(positionMs);
}

bool AudioEngine::contentLoopActive() const
{
    // Per-track clip loop only. Transport/song loop uses m_loopStartMs/m_loopEndMs.
    for (const AudioTrack &track : m_tracks) {
        if (track.loopEnabled) {
            return true;
        }
    }
    return false;
}

QVector<float> AudioEngine::trackSamplesAtSeconds(double seconds, bool /*loopContent*/) const
{
    QVector<float> samples(m_tracks.size());
    for (int i = 0; i < m_tracks.size(); ++i) {
        // Per-track only: AudioTrack::loopEnabled (+ loopStart/End). Do not force-loop all clips.
        samples[i] = m_tracks.at(i).sampleAtSeconds(seconds);
    }
    return samples;
}

QVector<float> AudioEngine::trackSamplesAtMs(int positionMs, bool loopContent) const
{
    return trackSamplesAtSeconds(static_cast<double>(positionMs) / 1000.0, loopContent);
}

namespace {

TrackProcessParams makeProcessParams(const AudioTrack &track, bool audible, double seconds)
{
    TrackProcessParams params;
    const float autoGain = track.automationVolumeAtSeconds(seconds);
    params.volume = std::clamp(track.volume * autoGain, 0.0f, 1.0f);
    params.pan = track.pan;
    params.audible = audible;
    params.eqLowDb = track.eqLowDb;
    params.eqMidDb = track.eqMidDb;
    params.eqHighDb = track.eqHighDb;
    const QVector<float> bands = track.eqBandsOrDefault();
    params.useGraphicEq = true;
    for (int b = 0; b < TrackProcessParams::kEqBandCount; ++b) {
        params.eqBands[b] = b < bands.size() ? bands.at(b) : 0.0f;
    }
    params.compThreshold = track.compThreshold;
    params.compRatio = track.compRatio;
    params.fxBypass = track.fxBypass;
    return params;
}

} // namespace

StereoSample AudioEngine::renderMixAtSeconds(double seconds, bool loopContent) const
{
    StereoSample mix;
    const QVector<float> trackMonoSamples = trackSamplesAtSeconds(seconds, loopContent);
    const int count = std::min(trackMonoSamples.size(), static_cast<qsizetype>(m_tracks.size()));
    for (int i = 0; i < count; ++i) {
        const AudioTrack &track = m_tracks.at(i);
        const TrackProcessParams params = makeProcessParams(track, trackAudible(i), seconds);
        const StereoSample processed = DspProcessor::processTrackSample(trackMonoSamples.at(i), params);
        mix = DspProcessor::mixLinear(mix, processed);
    }
    return DspProcessor::applyMasterChain(mix, m_masterVolume);
}

StereoSample AudioEngine::renderMixAtMs(int positionMs, bool loopContent) const
{
    return renderMixAtSeconds(static_cast<double>(positionMs) / 1000.0, loopContent);
}

StereoSample AudioEngine::renderMixFrame(const QVector<float> &trackMonoSamples) const
{
    StereoSample mix;
    const int count = std::min(trackMonoSamples.size(), static_cast<qsizetype>(m_tracks.size()));
    for (int i = 0; i < count; ++i) {
        const AudioTrack &track = m_tracks.at(i);
        const TrackProcessParams params =
            makeProcessParams(track, trackAudible(i), static_cast<double>(m_positionMs) / 1000.0);
        const StereoSample processed = DspProcessor::processTrackSample(trackMonoSamples.at(i), params);
        mix = DspProcessor::mixLinear(mix, processed);
    }
    return DspProcessor::applyMasterChain(mix, m_masterVolume);
}
