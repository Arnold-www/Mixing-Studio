#include <ViewModel/RealMixerViewModel.h>

#include <Command/MixerCommands.h>
#include <Common/MixerTypes.h>
#include <Model/AudioEngine.h>
#include <Model/AudioTrack.h>
#include <ViewModel/TrackViewModel.h>

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QtAlgorithms>

#include <utility>

RealMixerViewModel::RealMixerViewModel(AudioEngine *engine, QObject *parent)
    : IMixerViewModel(parent)
    , m_engine(engine)
{
    Q_ASSERT(m_engine != nullptr);

    m_masterVolume = m_engine->masterVolume();
    ensureAssetLibrary();
    seedAssetLibraryIfEmpty();

    connect(m_engine, &AudioEngine::playbackStateChanged, this, &IMixerViewModel::playingChanged);
    connect(m_engine, &AudioEngine::statusMessageChanged, this, &RealMixerViewModel::setStatusMessage);
    connect(m_engine, &AudioEngine::positionChanged, this, &IMixerViewModel::playbackPositionChanged);
    connect(m_engine, &AudioEngine::durationChanged, this, &IMixerViewModel::durationChanged);
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

    return static_cast<float>(m_engine->positionMs()) / static_cast<float>(m_engine->durationMs());
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

void RealMixerViewModel::importMockTrack()
{
    if (!m_engine) {
        return;
    }

    ImportMockTrackCommand command(m_engine, m_tracks.size());
    command.execute();
    addTrack(command.trackName());
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
    pullAnalysisFromEngine();
    refreshFilteredAssetNames();
}

void RealMixerViewModel::saveProject()
{
    if (!m_engine) {
        return;
    }

    const QString fileName = QStringLiteral("Project %1 tracks.json").arg(m_tracks.size());
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
    qDeleteAll(m_tracks);
    m_tracks.clear();
    m_anySolo = false;
    emit tracksChanged();
    emit soloStateChanged();
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
        track->setVolume(data.volume);
        track->setPan(data.pan);
        track->setMuted(data.muted);
        track->setSolo(data.solo);
    }

    m_masterVolume = m_engine->masterVolume();
    emit masterVolumeChanged();
    refreshSoloState();
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
    params.muted = track->muted();
    params.solo = track->solo();
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
        item.muted = track->muted();
        item.solo = track->solo();
        params.append(item);
    }
    ApplyAllTrackDspCommand(m_engine, std::move(params)).execute();
}

void RealMixerViewModel::pullAnalysisFromEngine()
{
    if (!m_engine) {
        return;
    }

    QVariantList waveform;
    waveform.reserve(m_engine->waveformPoints().size());
    for (float value : m_engine->waveformPoints()) {
        waveform.append(value);
    }

    QVariantList spectrum;
    spectrum.reserve(m_engine->spectrumLevels().size());
    for (float value : m_engine->spectrumLevels()) {
        spectrum.append(value);
    }

    m_waveformPoints = waveform;
    m_spectrumLevels = spectrum;
    m_vuLevel = m_engine->vuLevel();
    m_peakLevel = m_engine->peakLevel();
    m_clippingDetected = m_engine->clippingDetected();

    emit waveformPointsChanged();
    emit spectrumLevelsChanged();
    emit analysisMetersChanged();

    for (int i = 0; i < m_tracks.size(); ++i) {
        const float level = m_vuLevel * m_tracks.at(i)->volume();
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
}

void RealMixerViewModel::seedAssetLibraryIfEmpty()
{
    if (!m_engine) {
        return;
    }

    if (!m_engine->recentAssets(1).isEmpty()) {
        return;
    }

    for (const QString &name : defaultAssetCatalog()) {
        m_engine->registerAsset(name, name);
    }
}

QString RealMixerViewModel::projectsDirectory() const
{
    const QString root = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(root).filePath(QStringLiteral("projects"));
}

QString RealMixerViewModel::projectFilePath(const QString &fileName) const
{
    return QDir(projectsDirectory()).filePath(fileName);
}
