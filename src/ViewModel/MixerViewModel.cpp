#include <ViewModel/MixerViewModel.h>

#include <App/MixerApp.h>
#include <Command/MixerCommands.h>
#include <ViewModel/TrackViewModel.h>

MixerViewModel::MixerViewModel(MixerApp *app, QObject *parent)
    : QObject(parent)
    , m_app(app)
{
    Q_ASSERT(m_app != nullptr);

    m_assetNames = m_app->defaultAssetCatalog();
    m_recentProjectNames = m_app->defaultRecentProjects();
    m_masterVolume = m_app->masterVolume();

    connect(m_app, &MixerApp::playbackStateChanged, this, &MixerViewModel::playingChanged);
    connect(m_app, &MixerApp::statusMessageChanged, this, &MixerViewModel::setStatusMessage);
    connect(m_app, &MixerApp::positionChanged, this, [this]() {
        emit playbackPositionChanged();
        updateMockAnalysisData();
    });
    connect(m_app, &MixerApp::durationChanged, this, &MixerViewModel::durationChanged);
    connect(m_app, &MixerApp::masterVolumeChanged, this, [this]() {
        if (!m_app) {
            return;
        }
        m_masterVolume = m_app->masterVolume();
        emit masterVolumeChanged();
    });

    // Presentation mock: fixed pretty waveform/spectrum while tracks remain mock PCM.
    m_playbackTimer.setInterval(200);
    connect(&m_playbackTimer, &QTimer::timeout, this, &MixerViewModel::updateMockAnalysisData);
    connect(m_app, &MixerApp::playbackStateChanged, this, &MixerViewModel::updatePlaybackTimer);

    updateMockAnalysisData();
    refreshFilteredAssetNames();
}

QQmlListProperty<TrackViewModel> MixerViewModel::tracks()
{
    return QQmlListProperty<TrackViewModel>(this, &m_tracks, &MixerViewModel::trackCount, &MixerViewModel::trackAt);
}

bool MixerViewModel::playing() const
{
    return m_app && m_app->isPlaying();
}

QString MixerViewModel::statusMessage() const
{
    return m_statusMessage;
}

float MixerViewModel::masterVolume() const
{
    return m_masterVolume;
}

int MixerViewModel::positionSeconds() const
{
    return m_app ? m_app->positionMs() / 1000 : 0;
}

int MixerViewModel::durationSeconds() const
{
    return m_app ? m_app->durationMs() / 1000 : 0;
}

float MixerViewModel::playbackProgress() const
{
    if (!m_app || m_app->durationMs() <= 0) {
        return 0.0f;
    }

    return static_cast<float>(m_app->positionMs()) / static_cast<float>(m_app->durationMs());
}

QString MixerViewModel::playbackTimeText() const
{
    return QStringLiteral("%1 / %2").arg(formatTime(positionSeconds()), formatTime(durationSeconds()));
}

bool MixerViewModel::anySolo() const
{
    return m_anySolo;
}

QVariantList MixerViewModel::waveformPoints() const
{
    return m_waveformPoints;
}

QVariantList MixerViewModel::spectrumLevels() const
{
    return m_spectrumLevels;
}

QString MixerViewModel::assetSearchText() const
{
    return m_assetSearchText;
}

QStringList MixerViewModel::filteredAssetNames() const
{
    return m_filteredAssetNames;
}

QStringList MixerViewModel::recentProjectNames() const
{
    return m_recentProjectNames;
}

void MixerViewModel::importMockTrack()
{
    if (!m_app) {
        return;
    }

    ImportMockTrackCommand command(m_app, m_tracks.size());
    command.execute();
    addTrack(command.trackName());
    updateMockAnalysisData();
}

void MixerViewModel::importAssetByName(const QString &name)
{
    if (!m_app) {
        return;
    }

    ImportAssetCommand command(m_app, name);
    command.execute();
    if (!command.ok()) {
        setStatusMessage(command.status());
        return;
    }

    addTrack(command.assetName());
    setStatusMessage(command.status());
    updateMockAnalysisData();
}

void MixerViewModel::restoreRecentProject(const QString &name)
{
    if (!m_app) {
        return;
    }

    RestoreRecentProjectCommand command(m_app, name);
    command.execute();
    setStatusMessage(command.status());
}

void MixerViewModel::saveMockProject()
{
    if (!m_app) {
        return;
    }

    SaveMockProjectCommand command(m_app, m_tracks.size(), m_recentProjectNames);
    command.execute();
    if (command.recentProjects() != m_recentProjectNames) {
        m_recentProjectNames = command.recentProjects();
        emit recentProjectNamesChanged();
    }
    setStatusMessage(command.status());
}

void MixerViewModel::play()
{
    PlayCommand(m_app).execute();
}

void MixerViewModel::pause()
{
    PauseCommand(m_app).execute();
}

void MixerViewModel::stop()
{
    StopCommand(m_app).execute();
}

void MixerViewModel::setMasterVolume(float volume)
{
    if (!m_app) {
        return;
    }

    SetMasterVolumeCommand command(m_app, volume);
    command.execute();
    const float clamped = command.clampedVolume();
    if (qFuzzyCompare(m_masterVolume, clamped)) {
        return;
    }

    m_masterVolume = clamped;
    emit masterVolumeChanged();
}

void MixerViewModel::seekToProgress(float progress)
{
    SeekProgressCommand(m_app, progress).execute();
}

void MixerViewModel::setAssetSearchText(const QString &text)
{
    if (m_assetSearchText == text) {
        return;
    }

    m_assetSearchText = text;
    emit assetSearchTextChanged();
    refreshFilteredAssetNames();
}

void MixerViewModel::setStatusMessage(const QString &message)
{
    if (m_statusMessage == message) {
        return;
    }

    m_statusMessage = message;
    emit statusMessageChanged();
}

void MixerViewModel::updatePlaybackTimer()
{
    if (playing()) {
        m_playbackTimer.start();
        return;
    }

    m_playbackTimer.stop();
}

QString MixerViewModel::formatTime(int seconds) const
{
    const int minutes = seconds / 60;
    const int remainingSeconds = seconds % 60;
    return QStringLiteral("%1:%2")
        .arg(minutes)
        .arg(remainingSeconds, 2, 10, QLatin1Char('0'));
}

qsizetype MixerViewModel::trackCount(QQmlListProperty<TrackViewModel> *property)
{
    const auto tracks = static_cast<QVector<TrackViewModel *> *>(property->data);
    return tracks->size();
}

TrackViewModel *MixerViewModel::trackAt(QQmlListProperty<TrackViewModel> *property, qsizetype index)
{
    const auto tracks = static_cast<QVector<TrackViewModel *> *>(property->data);
    return tracks->at(index);
}
