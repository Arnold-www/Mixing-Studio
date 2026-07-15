#pragma once

#include <ViewModel/IMixerViewModel.h>

#include <QStringList>
#include <QVariantList>
#include <QVector>

class AudioEngine;
class TrackViewModel;

// Concrete ViewModel. QML must only see IMixerViewModel* injected by App.
class RealMixerViewModel : public IMixerViewModel
{
    Q_OBJECT

public:
    explicit RealMixerViewModel(AudioEngine *engine, QObject *parent = nullptr);

    QQmlListProperty<ITrackViewModel> tracks() override;
    bool playing() const override;
    QString statusMessage() const override;
    float masterVolume() const override;
    int positionSeconds() const override;
    int durationSeconds() const override;
    float playbackProgress() const override;
    QString playbackTimeText() const override;
    bool anySolo() const override;
    QVariantList waveformPoints() const override;
    QVariantList spectrumLevels() const override;
    float vuLevel() const override;
    float peakLevel() const override;
    bool clippingDetected() const override;
    QString assetSearchText() const override;
    QStringList filteredAssetNames() const override;
    QStringList recentProjectNames() const override;

public slots:
    void importMockTrack() override;
    void importAssetByName(const QString &name) override;
    void restoreRecentProject(const QString &name) override;
    void saveProject() override;
    void play() override;
    void pause() override;
    void stop() override;
    void setMasterVolume(float volume) override;
    void seekToProgress(float progress) override;
    void setAssetSearchText(const QString &text) override;

private:
    void addTrack(const QString &name);
    void clearTracks();
    void rebuildTracksFromEngine();
    void setStatusMessage(const QString &message);
    void refreshSoloState();
    void syncTrackToEngine(int index);
    void syncAllTracksToEngine();
    void pullAnalysisFromEngine();
    void refreshFilteredAssetNames();
    void ensureAssetLibrary();
    void seedAssetLibraryIfEmpty();
    QString projectsDirectory() const;
    QString projectFilePath(const QString &fileName) const;
    QString formatTime(int seconds) const;

    static qsizetype trackCount(QQmlListProperty<ITrackViewModel> *property);
    static ITrackViewModel *trackAt(QQmlListProperty<ITrackViewModel> *property, qsizetype index);

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
};
