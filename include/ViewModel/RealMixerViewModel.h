#pragma once

#include <Common/MixerTypes.h>
#include <ViewModel/TrackViewModel.h>

#include <QObject>
#include <QQmlListProperty>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QVector>

class AudioEngine;

// Mixer ViewModel: business state + slots only. View wiring lives in MixingStudioViewBinder.
class RealMixerViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<TrackViewModel> tracks READ tracks NOTIFY tracksChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(float masterVolume READ masterVolume WRITE setMasterVolume NOTIFY masterVolumeChanged)
    Q_PROPERTY(float playbackProgress READ playbackProgress NOTIFY playbackPositionChanged)
    Q_PROPERTY(QString playbackTimeText READ playbackTimeText NOTIFY playbackPositionChanged)
    Q_PROPERTY(bool anySolo READ anySolo NOTIFY soloStateChanged)
    Q_PROPERTY(QVariantList waveformPoints READ waveformPoints NOTIFY waveformPointsChanged)
    Q_PROPERTY(QVariantList spectrumLevels READ spectrumLevels NOTIFY spectrumLevelsChanged)
    Q_PROPERTY(float vuLevel READ vuLevel NOTIFY analysisMetersChanged)
    Q_PROPERTY(QString assetSearchText READ assetSearchText WRITE setAssetSearchText NOTIFY assetSearchTextChanged)
    Q_PROPERTY(QStringList filteredAssetNames READ filteredAssetNames NOTIFY filteredAssetNamesChanged)
    Q_PROPERTY(QStringList recentProjectNames READ recentProjectNames NOTIFY recentProjectNamesChanged)
    Q_PROPERTY(bool loopEnabled READ loopEnabled WRITE setLoopEnabled NOTIFY loopRangeChanged)
    Q_PROPERTY(float loopStartProgress READ loopStartProgress NOTIFY loopRangeChanged)
    Q_PROPERTY(float loopEndProgress READ loopEndProgress NOTIFY loopRangeChanged)
    Q_PROPERTY(bool mockValidationMode READ mockValidationMode WRITE setMockValidationMode NOTIFY mockValidationModeChanged)
    Q_PROPERTY(int selectedTrackIndex READ selectedTrackIndex WRITE setSelectedTrackIndex NOTIFY selectedTrackIndexChanged)
    Q_PROPERTY(QVariantList automationPoints READ automationPoints NOTIFY automationPointsChanged)
    Q_PROPERTY(int selectedAssetIndex READ selectedAssetIndex WRITE setSelectedAssetIndex NOTIFY selectedAssetIndexChanged)
    Q_PROPERTY(int selectedRecentProjectIndex READ selectedRecentProjectIndex WRITE setSelectedRecentProjectIndex NOTIFY selectedRecentProjectIndexChanged)

public:
    explicit RealMixerViewModel(AudioEngine *engine, QObject *parent = nullptr);

    QQmlListProperty<TrackViewModel> tracks();
    QVariantList tracksAsObjects() const;
    bool playing() const;
    QString statusMessage() const;
    float masterVolume() const;
    float playbackProgress() const;
    QString playbackTimeText() const;
    bool anySolo() const;
    QVariantList waveformPoints() const;
    QVariantList spectrumLevels() const;
    float vuLevel() const;
    QString assetSearchText() const;
    QStringList filteredAssetNames() const;
    QStringList recentProjectNames() const;
    bool loopEnabled() const;
    float loopStartProgress() const;
    float loopEndProgress() const;
    bool mockValidationMode() const;
    int selectedTrackIndex() const;
    QVariantList automationPoints() const;
    int selectedAssetIndex() const;
    int selectedRecentProjectIndex() const;

public slots:
    void importMockTrack();
    void importLocalFile(const QString &pathOrUrl);
    void importAssetByName(const QString &name);
    void importSelectedAsset();
    void restoreRecentProject(const QString &name);
    void deleteRecentProject(const QString &name);
    void restoreSelectedRecentProject();
    void deleteSelectedRecentProject();
    void saveProject();
    void exportMix();
    void loadSampleProject();
    void play();
    void pause();
    void stop();
    void setMasterVolume(float volume);
    void seekToProgress(float progress);
    void setAssetSearchText(const QString &text);
    void setLoopEnabled(bool enabled);
    void setLoopRangeByProgress(float start, float end);
    void setMockValidationMode(bool enabled);
    void setSelectedTrackIndex(int index);
    void setSelectedAssetIndex(int index);
    void setSelectedRecentProjectIndex(int index);
    void addAutomationPoint(float progress, float value);
    void moveAutomationPoint(int pointIndex, float progress, float value);
    void clearAutomation();
    void deleteSelectedTrack();

signals:
    void tracksChanged();
    void playingChanged();
    void statusMessageChanged();
    void masterVolumeChanged();
    void playbackPositionChanged();
    void durationChanged();
    void soloStateChanged();
    void waveformPointsChanged();
    void spectrumLevelsChanged();
    void analysisMetersChanged();
    void assetSearchTextChanged();
    void filteredAssetNamesChanged();
    void recentProjectNamesChanged();
    void loopRangeChanged();
    void mockValidationModeChanged();
    void selectedTrackIndexChanged();
    void automationPointsChanged();
    void selectedAssetIndexChanged();
    void selectedRecentProjectIndexChanged();

private:
    void addTrack(const QString &name);
    void clearTracks();
    void rebuildTracksFromEngine();
    void setStatusMessage(const QString &message);
    void refreshSoloState();
    void syncTrackToEngine(int index);
    void syncAllTracksToEngine();
    void applyTrackDspToEngine(int index, const TrackDspParams &params);
    void applyLoadedProjectState();
    void pullAnalysisFromEngine();
    void applyMockAnalysisData();
    void refreshFilteredAssetNames();
    void ensureAssetLibrary();
    void seedSampleAssets();
    void refreshRecentProjectsFromDisk();
    void applyLoopToEngine();
    void refreshAutomationPoints();
    int positionSeconds() const;
    int durationSeconds() const;
    QString projectsDirectory() const;
    QString exportsDirectory() const;
    QString samplesDirectory() const;
    QString projectFilePath(const QString &fileName) const;
    QString formatTime(int seconds) const;

    static qsizetype trackCount(QQmlListProperty<TrackViewModel> *property);
    static TrackViewModel *trackAt(QQmlListProperty<TrackViewModel> *property, qsizetype index);

    AudioEngine *m_engine = nullptr;
    QVector<TrackViewModel *> m_tracks;
    QString m_statusMessage = QStringLiteral("Ready.");
    float m_masterVolume = 1.0f;
    bool m_anySolo = false;
    QVariantList m_waveformPoints;
    QVariantList m_spectrumLevels;
    float m_vuLevel = 0.0f;
    float m_peakLevel = 0.0f;
    bool m_clippingDetected = false;
    QString m_assetSearchText;
    QStringList m_filteredAssetNames;
    QStringList m_recentProjectNames;
    bool m_loopEnabled = false;
    float m_loopStartProgress = 0.0f;
    float m_loopEndProgress = 1.0f;
    bool m_mockValidationMode = false;
    int m_analysisFrame = 0;
    int m_selectedTrackIndex = -1;
    QVariantList m_automationPoints;
    int m_selectedAssetIndex = -1;
    int m_selectedRecentProjectIndex = -1;
};
