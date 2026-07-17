#include <ViewModel/RealMixerViewModel.h>

#include <Command/MixerCommands.h>
#include <Common/MixerTypes.h>
#include <Model/AudioEngine.h>
#include <Model/AudioTrack.h>
#include <ViewModel/TrackViewModel.h>

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QVariantMap>
#include <QtAlgorithms>

#include <algorithm>
#include <cmath>
#include <utility>

RealMixerViewModel::RealMixerViewModel(AudioEngine *engine, QObject *parent)
    : IMixerViewModel(parent)
    , m_engine(engine)
{
    Q_ASSERT(m_engine != nullptr);

    m_masterVolume = m_engine->masterVolume();
    ensureAssetLibrary();
    refreshRecentProjectsFromDisk();

    connect(m_engine, &AudioEngine::playbackStateChanged, this, &IMixerViewModel::playingChanged);
    connect(m_engine, &AudioEngine::statusMessageChanged, this, &RealMixerViewModel::setStatusMessage);
    connect(m_engine, &AudioEngine::positionChanged, this, &IMixerViewModel::playbackPositionChanged);
    connect(m_engine, &AudioEngine::durationChanged, this, [this]() {
        emit durationChanged();
        emit loopRangeChanged();
    });
    connect(m_engine, &AudioEngine::loopRangeChanged, this, &IMixerViewModel::loopRangeChanged);
    connect(m_engine, &AudioEngine::masterVolumeChanged, this, [this]() {
        if (!m_engine) {
            return;
        }
        m_masterVolume = m_engine->masterVolume();
        emit masterVolumeChanged();
    });
    connect(m_engine, &AudioEngine::analysisChanged, this, &RealMixerViewModel::pullAnalysisFromEngine);

    pullAnalysisFromEngine();
    refreshFilteredAssetNames();
}

QQmlListProperty<ITrackViewModel> RealMixerViewModel::tracks()
{
    return QQmlListProperty<ITrackViewModel>(this, &m_tracks, &RealMixerViewModel::trackCount, &RealMixerViewModel::trackAt);
}

bool RealMixerViewModel::playing() const
{
    return m_engine && m_engine->isPlaying();
}

QString RealMixerViewModel::statusMessage() const
{
    return m_statusMessage;
}

float RealMixerViewModel::masterVolume() const
{
    return m_masterVolume;
}

int RealMixerViewModel::positionSeconds() const
{
    return m_engine ? m_engine->positionMs() / 1000 : 0;
}

int RealMixerViewModel::durationSeconds() const
{
    return m_engine ? m_engine->durationMs() / 1000 : 0;
}

float RealMixerViewModel::playbackProgress() const
{
    if (!m_engine || m_engine->durationMs() <= 0) {
        return 0.0f;
    }

    // Sub-ms position keeps Seek/playhead smooth at ~120 Hz UI ticks.
    const double durationUs = static_cast<double>(m_engine->durationMs()) * 1000.0;
    return static_cast<float>(static_cast<double>(m_engine->positionUSecs()) / durationUs);
}

QString RealMixerViewModel::playbackTimeText() const
{
    return QStringLiteral("%1 / %2").arg(formatTime(positionSeconds()), formatTime(durationSeconds()));
}

bool RealMixerViewModel::anySolo() const
{
    return m_anySolo;
}

QVariantList RealMixerViewModel::waveformPoints() const
{
    return m_waveformPoints;
}

QVariantList RealMixerViewModel::spectrumLevels() const
{
    return m_spectrumLevels;
}

float RealMixerViewModel::vuLevel() const
{
    return m_vuLevel;
}

float RealMixerViewModel::peakLevel() const
{
    return m_peakLevel;
}

bool RealMixerViewModel::clippingDetected() const
{
    return m_clippingDetected;
}

QString RealMixerViewModel::assetSearchText() const
{
    return m_assetSearchText;
}

QStringList RealMixerViewModel::filteredAssetNames() const
{
    return m_filteredAssetNames;
}

QStringList RealMixerViewModel::recentProjectNames() const
{
    return m_recentProjectNames;
}

bool RealMixerViewModel::loopEnabled() const
{
    return m_loopEnabled;
}

float RealMixerViewModel::loopStartProgress() const
{
    return m_loopStartProgress;
}

float RealMixerViewModel::loopEndProgress() const
{
    return m_loopEndProgress;
}

bool RealMixerViewModel::mockValidationMode() const
{
    return m_mockValidationMode;
}

int RealMixerViewModel::selectedTrackIndex() const
{
    return m_selectedTrackIndex;
}

QVariantList RealMixerViewModel::automationPoints() const
{
    return m_automationPoints;
}

void RealMixerViewModel::importMockTrack()
{
    if (!m_engine) {
        return;
    }

    ImportMockTrackCommand command(m_engine, m_tracks.size());
    command.execute();
    addTrack(command.trackName());
}

void RealMixerViewModel::importLocalFile(const QString &pathOrUrl)
{
    if (!m_engine) {
        return;
    }

    ImportLocalFileCommand command(m_engine, pathOrUrl);
    command.execute();
    setStatusMessage(command.status());
    if (!command.ok()) {
        return;
    }

    addTrack(command.displayName());
    refreshFilteredAssetNames();
}

void RealMixerViewModel::importAssetByName(const QString &name)
{
    if (!m_engine) {
        return;
    }

    ImportAssetCommand command(m_engine, name);
    command.execute();
    if (!command.ok()) {
        setStatusMessage(command.status());
        return;
    }

    addTrack(command.assetName());
    setStatusMessage(command.status());
    refreshFilteredAssetNames();
}

void RealMixerViewModel::restoreRecentProject(const QString &name)
{
    if (!m_engine) {
        return;
    }

    LoadProjectCommand command(m_engine, projectFilePath(name));
    command.execute();
    setStatusMessage(command.status());
    if (!command.ok()) {
        return;
    }

    rebuildTracksFromEngine();
    m_loopEnabled = m_engine->loopEndMs() > m_engine->loopStartMs();
    if (m_engine->durationMs() > 0) {
        m_loopStartProgress = static_cast<float>(m_engine->loopStartMs()) / static_cast<float>(m_engine->durationMs());
        m_loopEndProgress = static_cast<float>(m_engine->loopEndMs()) / static_cast<float>(m_engine->durationMs());
    }
    emit loopRangeChanged();
    pullAnalysisFromEngine();
    refreshFilteredAssetNames();
}

void RealMixerViewModel::deleteRecentProject(const QString &name)
{
    if (name.trimmed().isEmpty()) {
        setStatusMessage(QStringLiteral("Select a project to delete."));
        return;
    }

    const QString path = projectFilePath(name);
    if (!QFileInfo::exists(path)) {
        m_recentProjectNames.removeAll(name);
        emit recentProjectNamesChanged();
        setStatusMessage(QStringLiteral("Project already missing: %1").arg(name));
        return;
    }

    if (!QFile::remove(path)) {
        setStatusMessage(QStringLiteral("Failed to delete project: %1").arg(name));
        return;
    }

    m_recentProjectNames.removeAll(name);
    emit recentProjectNamesChanged();
    refreshRecentProjectsFromDisk();
    setStatusMessage(QStringLiteral("Deleted project: %1").arg(name));
}

void RealMixerViewModel::saveProject()
{
    if (!m_engine) {
        return;
    }

    // There is no current-project identity yet, so a track-count-based name
    // silently overwrote unrelated sessions with the same number of tracks.
    const QString fileName = QStringLiteral("Project %1 tracks %2.json")
                                 .arg(m_tracks.size())
                                 .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz")));
    const QString path = projectFilePath(fileName);
    QDir().mkpath(projectsDirectory());

    SaveProjectCommand command(m_engine, path);
    command.execute();
    setStatusMessage(command.status());
    if (!command.ok()) {
        return;
    }

    m_recentProjectNames.removeAll(fileName);
    m_recentProjectNames.prepend(fileName);
    while (m_recentProjectNames.size() > 5) {
        m_recentProjectNames.removeLast();
    }
    emit recentProjectNamesChanged();
}

void RealMixerViewModel::exportMix()
{
    if (!m_engine) {
        return;
    }

    QDir().mkpath(exportsDirectory());
    const QString path = QDir(exportsDirectory()).filePath(QStringLiteral("mix_export.wav"));
    ExportMixCommand command(m_engine, path, 3000);
    command.execute();
    setStatusMessage(command.status());
}

void RealMixerViewModel::loadSampleProject()
{
    if (!m_engine) {
        return;
    }

    const QString sampleJson = QDir(samplesDirectory()).filePath(QStringLiteral("demo_session.json"));
    if (!QFileInfo::exists(sampleJson)) {
        setStatusMessage(QStringLiteral("Sample project not found: %1").arg(sampleJson));
        return;
    }

    LoadProjectCommand command(m_engine, sampleJson);
    command.execute();
    setStatusMessage(command.status());
    if (!command.ok()) {
        return;
    }

    rebuildTracksFromEngine();
    m_loopEnabled = m_engine->loopEndMs() > m_engine->loopStartMs();
    if (m_engine->durationMs() > 0) {
        m_loopStartProgress = static_cast<float>(m_engine->loopStartMs()) / static_cast<float>(m_engine->durationMs());
        m_loopEndProgress = static_cast<float>(m_engine->loopEndMs()) / static_cast<float>(m_engine->durationMs());
    }
    emit loopRangeChanged();
    pullAnalysisFromEngine();
    refreshFilteredAssetNames();
}

void RealMixerViewModel::play()
{
    PlayCommand(m_engine).execute();
}

void RealMixerViewModel::pause()
{
    PauseCommand(m_engine).execute();
}

void RealMixerViewModel::stop()
{
    StopCommand(m_engine).execute();
}

void RealMixerViewModel::setMasterVolume(float volume)
{
    if (!m_engine) {
        return;
    }

    SetMasterVolumeCommand command(m_engine, volume);
    command.execute();
    const float clamped = command.clampedVolume();
    if (qFuzzyCompare(m_masterVolume, clamped)) {
        return;
    }

    m_masterVolume = clamped;
    emit masterVolumeChanged();
}

void RealMixerViewModel::seekToProgress(float progress)
{
    SeekProgressCommand(m_engine, progress).execute();
}

void RealMixerViewModel::setAssetSearchText(const QString &text)
{
    if (m_assetSearchText == text) {
        return;
    }

    m_assetSearchText = text;
    emit assetSearchTextChanged();
    refreshFilteredAssetNames();
}

void RealMixerViewModel::setLoopEnabled(bool enabled)
{
    if (m_loopEnabled == enabled) {
        return;
    }

    m_loopEnabled = enabled;
    if (m_loopEnabled && m_loopEndProgress <= m_loopStartProgress) {
        m_loopStartProgress = 0.0f;
        m_loopEndProgress = 1.0f;
    }
    applyLoopToEngine();
    emit loopRangeChanged();
    setStatusMessage(m_loopEnabled ? QStringLiteral("Transport loop on (song timeline).")
                                   : QStringLiteral("Transport loop off."));
}

void RealMixerViewModel::setLoopRangeByProgress(float start, float end)
{
    float s = std::clamp(start, 0.0f, 1.0f);
    float e = std::clamp(end, 0.0f, 1.0f);
    if (e < s) {
        std::swap(s, e);
    }
    if (qFuzzyCompare(m_loopStartProgress, s) && qFuzzyCompare(m_loopEndProgress, e)) {
        return;
    }

    m_loopStartProgress = s;
    m_loopEndProgress = e;
    if (m_loopEnabled) {
        applyLoopToEngine();
    }
    emit loopRangeChanged();
}

void RealMixerViewModel::setMockValidationMode(bool enabled)
{
    if (m_mockValidationMode == enabled) {
        return;
    }

    m_mockValidationMode = enabled;
    emit mockValidationModeChanged();
    if (m_mockValidationMode) {
        applyMockAnalysisData();
        setStatusMessage(QStringLiteral("Mock validation mode enabled."));
    } else {
        pullAnalysisFromEngine();
        setStatusMessage(QStringLiteral("Mock validation mode disabled."));
    }
}

void RealMixerViewModel::setStatusMessage(const QString &message)
{
    if (m_statusMessage == message) {
        return;
    }

    m_statusMessage = message;
    emit statusMessageChanged();
}

QString RealMixerViewModel::formatTime(int seconds) const
{
    const int minutes = seconds / 60;
    const int remainingSeconds = seconds % 60;
    return QStringLiteral("%1:%2")
        .arg(minutes)
        .arg(remainingSeconds, 2, 10, QLatin1Char('0'));
}

qsizetype RealMixerViewModel::trackCount(QQmlListProperty<ITrackViewModel> *property)
{
    const auto tracks = static_cast<QVector<TrackViewModel *> *>(property->data);
    return tracks->size();
}

ITrackViewModel *RealMixerViewModel::trackAt(QQmlListProperty<ITrackViewModel> *property, qsizetype index)
{
    const auto tracks = static_cast<QVector<TrackViewModel *> *>(property->data);
    return tracks->at(index);
}

void RealMixerViewModel::addTrack(const QString &name)
{
    auto *track = new TrackViewModel(name, this);
    const int index = m_tracks.size();
    connect(track, &TrackViewModel::soloChanged, this, &RealMixerViewModel::refreshSoloState);
    connect(track, &TrackViewModel::dspParamsChanged, this, [this, index]() {
        syncTrackToEngine(index);
    });
    m_tracks.append(track);
    refreshSoloState();
    syncTrackToEngine(index);
    emit tracksChanged();
}

void RealMixerViewModel::clearTracks()
{
    // Drop list membership and notify QML before deleting objects so ListView
    // delegates release bindings while pointers are still valid, then destroy.
    QVector<TrackViewModel *> doomed = std::move(m_tracks);
    m_tracks.clear();
    m_anySolo = false;
    emit tracksChanged();
    emit soloStateChanged();
    qDeleteAll(doomed);
}

void RealMixerViewModel::rebuildTracksFromEngine()
{
    clearTracks();
    if (!m_engine) {
        return;
    }

    const QList<AudioTrack> trackData = m_engine->trackData();
    for (int i = 0; i < trackData.size(); ++i) {
        const AudioTrack &data = trackData.at(i);
        auto *track = new TrackViewModel(data.displayName, this);
        connect(track, &TrackViewModel::soloChanged, this, &RealMixerViewModel::refreshSoloState);
        connect(track, &TrackViewModel::dspParamsChanged, this, [this, i]() {
            syncTrackToEngine(i);
        });
        m_tracks.append(track);
        track->applyLoadedDsp(data.volume,
                              data.pan,
                              data.muted,
                              data.solo,
                              data.eqLowDb,
                              data.eqMidDb,
                              data.eqHighDb,
                              data.compThreshold,
                              data.compRatio,
                              data.fxBypass,
                              data.eqBandsOrDefault(),
                              data.playbackRate,
                              data.loopEnabled,
                              data.loopStart,
                              data.loopEnd);
    }

    m_masterVolume = m_engine->masterVolume();
    emit masterVolumeChanged();
    refreshSoloState();
    if (m_selectedTrackIndex >= m_tracks.size()) {
        m_selectedTrackIndex = m_tracks.isEmpty() ? -1 : 0;
        emit selectedTrackIndexChanged();
    } else if (m_selectedTrackIndex < 0 && !m_tracks.isEmpty()) {
        m_selectedTrackIndex = 0;
        emit selectedTrackIndexChanged();
    }
    refreshAutomationPoints();
    emit tracksChanged();
    emit durationChanged();
    emit playbackPositionChanged();
}

void RealMixerViewModel::refreshSoloState()
{
    QVector<bool> soloFlags;
    soloFlags.reserve(m_tracks.size());
    for (const TrackViewModel *track : std::as_const(m_tracks)) {
        soloFlags.append(track->solo());
    }

    const SoloPlan plan = planSolo(soloFlags);
    const bool previousAnySolo = m_anySolo;
    m_anySolo = plan.anySolo;

    for (int i = 0; i < m_tracks.size(); ++i) {
        m_tracks.at(i)->setBlockedBySolo(plan.blockedBySolo.at(i));
    }

    syncAllTracksToEngine();

    if (previousAnySolo != m_anySolo) {
        emit soloStateChanged();
        setStatusMessage(m_anySolo ? QStringLiteral("Solo monitoring enabled.")
                                   : QStringLiteral("Solo monitoring cleared."));
    }
}

void RealMixerViewModel::syncTrackToEngine(int index)
{
    if (!m_engine || index < 0 || index >= m_tracks.size()) {
        return;
    }

    const TrackViewModel *track = m_tracks.at(index);
    TrackDspParams params;
    params.volume = track->volume();
    params.pan = track->pan();
    params.playbackRate = track->playbackRate();
    params.muted = track->muted();
    params.solo = track->solo();
    params.eqLowDb = track->eqLowDb();
    params.eqMidDb = track->eqMidDb();
    params.eqHighDb = track->eqHighDb();
    params.eqBands = track->eqBands();
    params.compThreshold = track->compThreshold();
    params.compRatio = track->compRatio();
    params.fxBypass = track->fxBypass();
    params.loopEnabled = track->loopEnabled();
    params.loopStart = track->loopStartProgress();
    params.loopEnd = track->loopEndProgress();
    ApplyTrackDspCommand(m_engine, index, params).execute();
}

void RealMixerViewModel::syncAllTracksToEngine()
{
    if (!m_engine) {
        return;
    }

    QVector<TrackDspParams> params;
    params.reserve(m_tracks.size());
    for (const TrackViewModel *track : std::as_const(m_tracks)) {
        TrackDspParams item;
        item.volume = track->volume();
        item.pan = track->pan();
        item.playbackRate = track->playbackRate();
        item.muted = track->muted();
        item.solo = track->solo();
        item.eqLowDb = track->eqLowDb();
        item.eqMidDb = track->eqMidDb();
        item.eqHighDb = track->eqHighDb();
        item.eqBands = track->eqBands();
        item.compThreshold = track->compThreshold();
        item.compRatio = track->compRatio();
        item.fxBypass = track->fxBypass();
        item.loopEnabled = track->loopEnabled();
        item.loopStart = track->loopStartProgress();
        item.loopEnd = track->loopEndProgress();
        params.append(item);
    }
    ApplyAllTrackDspCommand(m_engine, std::move(params)).execute();
}

void RealMixerViewModel::pullAnalysisFromEngine()
{
    if (!m_engine) {
        return;
    }

    if (m_mockValidationMode) {
        applyMockAnalysisData();
        return;
    }

    const QVector<float> waveformSrc = m_engine->waveformPoints();
    const QVector<float> spectrumSrc = m_engine->spectrumLevels();
    const bool playing = m_engine->isPlaying();

    // EMA toward engine samples keeps the UI smooth at ~60 Hz without stair-steps.
    const float waveAlpha = playing ? 0.42f : 1.0f;
    const float specAttack = playing ? 0.62f : 1.0f;
    const float specRelease = playing ? 0.28f : 1.0f;

    QVariantList waveform;
    waveform.reserve(waveformSrc.size());
    for (int i = 0; i < waveformSrc.size(); ++i) {
        const float target = waveformSrc.at(i);
        const float prev = (i < m_waveformPoints.size()) ? m_waveformPoints.at(i).toFloat() : target;
        waveform.append(prev + (target - prev) * waveAlpha);
    }

    QVariantList spectrum;
    spectrum.reserve(spectrumSrc.size());
    for (int i = 0; i < spectrumSrc.size(); ++i) {
        const float target = spectrumSrc.at(i);
        const float prev = (i < m_spectrumLevels.size()) ? m_spectrumLevels.at(i).toFloat() : target;
        const float alpha = (target >= prev) ? specAttack : specRelease;
        spectrum.append(prev + (target - prev) * alpha);
    }

    const bool waveformChanged = playing || m_waveformPoints != waveform;
    const bool spectrumChanged = playing || m_spectrumLevels != spectrum;

    if (waveformChanged) {
        m_waveformPoints = waveform;
        emit waveformPointsChanged();
    }

    if (spectrumChanged) {
        m_spectrumLevels = spectrum;
        emit spectrumLevelsChanged();
    }

    const float nextVu = m_engine->vuLevel();
    const float nextPeak = m_engine->peakLevel();
    const bool nextClip = m_engine->clippingDetected();
    const bool metersChanged = playing
        || std::fabs(m_vuLevel - nextVu) > 0.004f
        || std::fabs(m_peakLevel - nextPeak) > 0.004f
        || m_clippingDetected != nextClip;

    m_vuLevel = nextVu;
    m_peakLevel = nextPeak;
    m_clippingDetected = nextClip;
    if (metersChanged) {
        emit analysisMetersChanged();
    }

    for (int i = 0; i < m_tracks.size(); ++i) {
        // Mix VU already includes fader gain — do not multiply volume again
        // (that made loud tracks read as a tiny meter segment).
        const float level = m_tracks.at(i)->audible() ? m_vuLevel : 0.0f;
        m_tracks.at(i)->setMeterLevel(level);
    }
}

void RealMixerViewModel::applyMockAnalysisData()
{
    ++m_analysisFrame;

    QVariantList waveform;
    waveform.reserve(256);
    for (int i = 0; i < 256; ++i) {
        const double phase = (m_analysisFrame * 0.18) + (i * 0.34);
        const double envelope = 0.35 + (0.45 * std::sin((i + m_analysisFrame) * 0.07));
        const double value = std::sin(phase) * envelope;
        waveform.append(std::clamp(value, -1.0, 1.0));
    }

    QVariantList spectrum;
    spectrum.reserve(32);
    for (int i = 0; i < 32; ++i) {
        const double phase = (m_analysisFrame * 0.11) + (i * 0.42);
        const double rolloff = 1.0 - (static_cast<double>(i) / 40.0);
        const double value = (0.22 + (0.58 * std::abs(std::sin(phase)))) * rolloff;
        spectrum.append(std::clamp(value, 0.0, 1.0));
    }

    m_waveformPoints = waveform;
    m_spectrumLevels = spectrum;
    m_vuLevel = 0.45f + 0.35f * static_cast<float>(std::abs(std::sin(m_analysisFrame * 0.09)));
    m_peakLevel = std::min(1.0f, m_vuLevel + 0.15f);
    m_clippingDetected = false;

    emit waveformPointsChanged();
    emit spectrumLevelsChanged();
    emit analysisMetersChanged();

    for (int i = 0; i < m_tracks.size(); ++i) {
        const float level = m_tracks.at(i)->audible() ? m_vuLevel : 0.0f;
        m_tracks.at(i)->setMeterLevel(level);
    }
}

void RealMixerViewModel::refreshFilteredAssetNames()
{
    if (!m_engine) {
        return;
    }

    const QStringList filtered = m_engine->searchAssets(m_assetSearchText);
    if (m_filteredAssetNames == filtered) {
        return;
    }

    m_filteredAssetNames = filtered;
    emit filteredAssetNamesChanged();
}

void RealMixerViewModel::ensureAssetLibrary()
{
    if (!m_engine) {
        return;
    }

    const QString root = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(root);
    const QString dbPath = QDir(root).filePath(QStringLiteral("mixing_studio_assets.db"));
    m_engine->openAssetLibrary(dbPath);
    seedSampleAssets();
    refreshFilteredAssetNames();
}

void RealMixerViewModel::seedSampleAssets()
{
    if (!m_engine) {
        return;
    }

    const QDir dir(samplesDirectory());
    if (!dir.exists()) {
        return;
    }

    const QFileInfoList files = dir.entryInfoList({QStringLiteral("*.wav")}, QDir::Files, QDir::Name);
    for (const QFileInfo &info : files) {
        m_engine->registerAsset(info.absoluteFilePath(), info.fileName());
    }
}

void RealMixerViewModel::setSelectedTrackIndex(int index)
{
    const int clamped = (index < 0 || index >= m_tracks.size()) ? -1 : index;
    if (m_selectedTrackIndex == clamped) {
        return;
    }
    m_selectedTrackIndex = clamped;
    emit selectedTrackIndexChanged();
    refreshAutomationPoints();
}

void RealMixerViewModel::refreshAutomationPoints()
{
    QVariantList points;
    if (m_engine && m_selectedTrackIndex >= 0 && m_selectedTrackIndex < m_engine->trackCount()) {
        const QList<AudioTrack> tracks = m_engine->trackData();
        if (m_selectedTrackIndex < tracks.size()) {
            const int duration = std::max(1, m_engine->durationMs());
            for (const AutomationPoint &point : tracks.at(m_selectedTrackIndex).volumeAutomation) {
                QVariantMap item;
                item.insert(QStringLiteral("progress"),
                            static_cast<float>(point.timeMs) / static_cast<float>(duration));
                item.insert(QStringLiteral("value"), point.value);
                points.append(item);
            }
        }
    }

    if (points == m_automationPoints) {
        return;
    }
    m_automationPoints = points;
    emit automationPointsChanged();
}

void RealMixerViewModel::addAutomationPoint(float progress, float value)
{
    if (!m_engine || m_selectedTrackIndex < 0 || m_selectedTrackIndex >= m_engine->trackCount()) {
        setStatusMessage(QStringLiteral("Select a track before editing automation."));
        return;
    }

    const int duration = std::max(1, m_engine->durationMs());
    QVector<AutomationPoint> points = m_engine->trackData().at(m_selectedTrackIndex).volumeAutomation;
    AutomationPoint point;
    point.timeMs = static_cast<int>(std::clamp(progress, 0.0f, 1.0f) * static_cast<float>(duration));
    point.value = std::clamp(value, 0.0f, 1.0f);
    points.append(point);
    m_engine->setTrackVolumeAutomation(m_selectedTrackIndex, points);
    refreshAutomationPoints();
    setStatusMessage(QStringLiteral("Automation point added on track %1.").arg(m_selectedTrackIndex + 1));
}

void RealMixerViewModel::moveAutomationPoint(int pointIndex, float progress, float value)
{
    if (!m_engine || m_selectedTrackIndex < 0 || m_selectedTrackIndex >= m_engine->trackCount()) {
        return;
    }

    QVector<AutomationPoint> points = m_engine->trackData().at(m_selectedTrackIndex).volumeAutomation;
    if (pointIndex < 0 || pointIndex >= points.size()) {
        return;
    }

    const int duration = std::max(1, m_engine->durationMs());
    points[pointIndex].timeMs = static_cast<int>(std::clamp(progress, 0.0f, 1.0f) * static_cast<float>(duration));
    points[pointIndex].value = std::clamp(value, 0.0f, 1.0f);
    m_engine->setTrackVolumeAutomation(m_selectedTrackIndex, points);
    refreshAutomationPoints();
}

void RealMixerViewModel::clearAutomation()
{
    if (!m_engine || m_selectedTrackIndex < 0 || m_selectedTrackIndex >= m_engine->trackCount()) {
        setStatusMessage(QStringLiteral("Select a track before clearing automation."));
        return;
    }

    m_engine->setTrackVolumeAutomation(m_selectedTrackIndex, {});
    refreshAutomationPoints();
    setStatusMessage(QStringLiteral("Automation cleared on track %1.").arg(m_selectedTrackIndex + 1));
}

void RealMixerViewModel::deleteSelectedTrack()
{
    if (!m_engine || m_selectedTrackIndex < 0 || m_selectedTrackIndex >= m_engine->trackCount()) {
        setStatusMessage(QStringLiteral("Select a track to delete."));
        return;
    }

    const int removedIndex = m_selectedTrackIndex;
    if (!m_engine->removeTrack(removedIndex)) {
        setStatusMessage(QStringLiteral("Failed to delete track."));
        return;
    }

    rebuildTracksFromEngine();
    if (m_tracks.isEmpty()) {
        m_selectedTrackIndex = -1;
    } else if (removedIndex >= m_tracks.size()) {
        m_selectedTrackIndex = m_tracks.size() - 1;
    } else {
        m_selectedTrackIndex = removedIndex;
    }
    emit selectedTrackIndexChanged();
    refreshAutomationPoints();
    pullAnalysisFromEngine();
    setStatusMessage(QStringLiteral("Track deleted."));
}

void RealMixerViewModel::refreshRecentProjectsFromDisk()
{
    QDir dir(projectsDirectory());
    if (!dir.exists()) {
        QDir().mkpath(projectsDirectory());
        m_recentProjectNames.clear();
        emit recentProjectNamesChanged();
        return;
    }

    QFileInfoList files = dir.entryInfoList({QStringLiteral("*.json")}, QDir::Files, QDir::Time);
    QStringList names;
    for (const QFileInfo &info : files) {
        names.append(info.fileName());
        if (names.size() >= 5) {
            break;
        }
    }

    if (names == m_recentProjectNames) {
        return;
    }
    m_recentProjectNames = names;
    emit recentProjectNamesChanged();
}

void RealMixerViewModel::applyLoopToEngine()
{
    if (!m_engine) {
        return;
    }

    if (!m_loopEnabled || m_engine->durationMs() <= 0) {
        m_engine->setLoopRange(0, 0);
        return;
    }

    const int startMs = static_cast<int>(m_loopStartProgress * static_cast<float>(m_engine->durationMs()));
    const int endMs = static_cast<int>(m_loopEndProgress * static_cast<float>(m_engine->durationMs()));
    m_engine->setLoopRange(startMs, endMs);
}

QString RealMixerViewModel::projectsDirectory() const
{
    const QString root = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(root).filePath(QStringLiteral("projects"));
}

QString RealMixerViewModel::exportsDirectory() const
{
    const QString root = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(root).filePath(QStringLiteral("exports"));
}

QString RealMixerViewModel::samplesDirectory() const
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        QDir(appDir).filePath(QStringLiteral("samples")),
        QDir(appDir).filePath(QStringLiteral("../samples")),
        QDir(appDir).filePath(QStringLiteral("../../samples")),
        QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("../../../samples")),
    };
    for (const QString &candidate : candidates) {
        const QString clean = QDir::cleanPath(candidate);
        if (QDir(clean).exists()) {
            return clean;
        }
    }
    return QDir(appDir).filePath(QStringLiteral("samples"));
}

QString RealMixerViewModel::projectFilePath(const QString &fileName) const
{
    return QDir(projectsDirectory()).filePath(fileName);
}
