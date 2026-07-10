#include <ViewModel/MixerViewModel.h>

#include <Model/AudioEngine.h>
#include <ViewModel/TrackViewModel.h>

#include <algorithm>
#include <cmath>
#include <utility>

MixerViewModel::MixerViewModel(AudioEngine *audioEngine, QObject *parent)
    : QObject(parent)
    , m_audioEngine(audioEngine)
    , m_assetNames({
          QStringLiteral("Lead Vocal.wav"),
          QStringLiteral("Backing Vocals.wav"),
          QStringLiteral("Acoustic Guitar.wav"),
          QStringLiteral("Electric Guitar DI.wav"),
          QStringLiteral("Bass DI.wav"),
          QStringLiteral("Kick Close.wav"),
          QStringLiteral("Snare Top.wav"),
          QStringLiteral("Drum Overheads.wav"),
          QStringLiteral("Room Ambience.wav"),
          QStringLiteral("Reference Mix.wav"),
      })
    , m_recentProjectNames({
          QStringLiteral("Studio Demo Session"),
          QStringLiteral("Podcast Voice Cleanup"),
          QStringLiteral("Band Rehearsal Mix"),
      })
{
    connect(m_audioEngine, &AudioEngine::playbackStateChanged, this, &MixerViewModel::playingChanged);
    connect(m_audioEngine, &AudioEngine::playbackStateChanged, this, &MixerViewModel::updatePlaybackTimer);
    connect(m_audioEngine, &AudioEngine::statusMessageChanged, this, &MixerViewModel::setStatusMessage);

    m_playbackTimer.setInterval(1000);
    connect(&m_playbackTimer, &QTimer::timeout, this, [this]() {
        if (m_positionSeconds >= m_durationSeconds) {
            stop();
            return;
        }

        setPositionSeconds(m_positionSeconds + 1);
        updateMockAnalysisData();
    });

    updateMockAnalysisData();
    refreshFilteredAssetNames();
}

QQmlListProperty<TrackViewModel> MixerViewModel::tracks()
{
    return QQmlListProperty<TrackViewModel>(this, &m_tracks, &MixerViewModel::trackCount, &MixerViewModel::trackAt);
}

bool MixerViewModel::playing() const
{
    return m_audioEngine->isPlaying();
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
    return m_positionSeconds;
}

int MixerViewModel::durationSeconds() const
{
    return m_durationSeconds;
}

float MixerViewModel::playbackProgress() const
{
    if (m_durationSeconds <= 0) {
        return 0.0f;
    }

    return static_cast<float>(m_positionSeconds) / static_cast<float>(m_durationSeconds);
}

QString MixerViewModel::playbackTimeText() const
{
    return QStringLiteral("%1 / %2").arg(formatTime(m_positionSeconds), formatTime(m_durationSeconds));
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
    const QString name = QStringLiteral("Track %1").arg(m_tracks.size() + 1);
    m_audioEngine->importTrack(name);
    addTrack(name);
    updateMockAnalysisData();
}

void MixerViewModel::importAssetByName(const QString &name)
{
    if (name.trimmed().isEmpty()) {
        setStatusMessage(QStringLiteral("Select an asset before importing."));
        return;
    }

    m_audioEngine->importTrack(name);
    addTrack(name);
    updateMockAnalysisData();
}

void MixerViewModel::restoreRecentProject(const QString &name)
{
    if (name.trimmed().isEmpty()) {
        setStatusMessage(QStringLiteral("Select a recent project before restore."));
        return;
    }

    setStatusMessage(QStringLiteral("Restore project queued: %1").arg(name));
}

void MixerViewModel::saveMockProject()
{
    if (m_tracks.isEmpty()) {
        setStatusMessage(QStringLiteral("Add at least one track before saving a project snapshot."));
        return;
    }

    const QString name = QStringLiteral("Mock Project %1 tracks").arg(m_tracks.size());
    m_recentProjectNames.removeAll(name);
    m_recentProjectNames.prepend(name);
    while (m_recentProjectNames.size() > 5) {
        m_recentProjectNames.removeLast();
    }

    emit recentProjectNamesChanged();
    setStatusMessage(QStringLiteral("Project snapshot queued: %1").arg(name));
}

void MixerViewModel::play()
{
    m_audioEngine->play();
}

void MixerViewModel::pause()
{
    m_audioEngine->pause();
}

void MixerViewModel::stop()
{
    m_audioEngine->stop();
    setPositionSeconds(0);
}

void MixerViewModel::setMasterVolume(float volume)
{
    const float clamped = std::clamp(volume, 0.0f, 1.0f);
    if (qFuzzyCompare(m_masterVolume, clamped)) {
        return;
    }

    m_masterVolume = clamped;
    m_audioEngine->setMasterVolume(m_masterVolume);
    emit masterVolumeChanged();
}

void MixerViewModel::seekToProgress(float progress)
{
    const float clamped = std::clamp(progress, 0.0f, 1.0f);
    setPositionSeconds(static_cast<int>(clamped * m_durationSeconds));
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

void MixerViewModel::addTrack(const QString &name)
{
    auto *track = new TrackViewModel(name, this);
    connect(track, &TrackViewModel::soloChanged, this, &MixerViewModel::refreshSoloState);
    m_tracks.append(track);
    refreshSoloState();
    emit tracksChanged();
}

void MixerViewModel::setStatusMessage(const QString &message)
{
    if (m_statusMessage == message) {
        return;
    }

    m_statusMessage = message;
    emit statusMessageChanged();
}

void MixerViewModel::setPositionSeconds(int positionSeconds)
{
    const int clamped = std::clamp(positionSeconds, 0, m_durationSeconds);
    if (m_positionSeconds == clamped) {
        return;
    }

    m_positionSeconds = clamped;
    emit playbackPositionChanged();
}

void MixerViewModel::updatePlaybackTimer()
{
    if (playing()) {
        m_playbackTimer.start();
        return;
    }

    m_playbackTimer.stop();
}

void MixerViewModel::refreshSoloState()
{
    const bool previousAnySolo = m_anySolo;
    m_anySolo = std::any_of(m_tracks.cbegin(), m_tracks.cend(), [](const TrackViewModel *track) {
        return track->solo();
    });

    for (auto *track : m_tracks) {
        track->setBlockedBySolo(m_anySolo && !track->solo());
    }

    if (previousAnySolo != m_anySolo) {
        emit soloStateChanged();
        setStatusMessage(m_anySolo ? QStringLiteral("Solo monitoring enabled.")
                                   : QStringLiteral("Solo monitoring cleared."));
    }
}

void MixerViewModel::updateMockAnalysisData()
{
    ++m_analysisFrame;

    QVariantList waveform;
    waveform.reserve(64);
    for (int i = 0; i < 64; ++i) {
        const double phase = (m_analysisFrame * 0.18) + (i * 0.34);
        const double envelope = 0.35 + (0.45 * std::sin((i + m_analysisFrame) * 0.07));
        const double value = std::sin(phase) * envelope;
        waveform.append(std::clamp(value, -1.0, 1.0));
    }

    QVariantList spectrum;
    spectrum.reserve(18);
    for (int i = 0; i < 18; ++i) {
        const double phase = (m_analysisFrame * 0.11) + (i * 0.42);
        const double rolloff = 1.0 - (static_cast<double>(i) / 24.0);
        const double value = (0.22 + (0.58 * std::abs(std::sin(phase)))) * rolloff;
        spectrum.append(std::clamp(value, 0.0, 1.0));
    }

    m_waveformPoints = waveform;
    m_spectrumLevels = spectrum;
    emit waveformPointsChanged();
    emit spectrumLevelsChanged();

    for (int i = 0; i < m_tracks.size(); ++i) {
        const double phase = (m_analysisFrame * 0.15) + (i * 0.8);
        const float level = static_cast<float>((0.28 + (0.56 * std::abs(std::sin(phase)))) * m_tracks.at(i)->volume());
        m_tracks.at(i)->setMeterLevel(level);
    }
}

void MixerViewModel::refreshFilteredAssetNames()
{
    QStringList filtered;
    for (const QString &name : std::as_const(m_assetNames)) {
        if (m_assetSearchText.isEmpty() || name.contains(m_assetSearchText, Qt::CaseInsensitive)) {
            filtered.append(name);
        }
    }

    if (m_filteredAssetNames == filtered) {
        return;
    }

    m_filteredAssetNames = filtered;
    emit filteredAssetNamesChanged();
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
