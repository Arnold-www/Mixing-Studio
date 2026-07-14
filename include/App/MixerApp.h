#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

class AudioEngine;

struct TrackDspParams
{
    float volume = 0.8f;
    float pan = 0.0f;
    bool muted = false;
    bool solo = false;
};

struct SoloPlan
{
    bool anySolo = false;
    QVector<bool> blockedBySolo;
};

// App / UseCase layer: orchestrates Model without QML or ViewModel types.
class MixerApp : public QObject
{
    Q_OBJECT

public:
    explicit MixerApp(AudioEngine *engine, QObject *parent = nullptr);

    AudioEngine *engine() const;

    bool isPlaying() const;
    int positionMs() const;
    int durationMs() const;
    float masterVolume() const;

    QVector<float> waveformPoints() const;
    QVector<float> spectrumLevels() const;
    float vuLevel() const;

    QStringList defaultAssetCatalog() const;
    QStringList defaultRecentProjects() const;
    QStringList filterAssets(const QStringList &catalog, const QString &query) const;

    static SoloPlan planSolo(const QVector<bool> &soloFlags);

public slots:
    void play();
    void pause();
    void stop();
    void seekToProgress(float progress);
    float setMasterVolume(float volume);

    QString importMockTrack(int existingTrackCount);
    bool importAssetByName(const QString &name, QString *statusOut = nullptr);

    void applyTrackDsp(int index, const TrackDspParams &params);
    void applyAllTrackDsp(const QVector<TrackDspParams> &params);

    QStringList saveMockSnapshot(int trackCount, QStringList recent, QString *statusOut = nullptr);
    QString restoreRecentProject(const QString &name, QString *statusOut = nullptr);

    void refreshAnalysis();

signals:
    void playbackStateChanged();
    void positionChanged();
    void durationChanged();
    void masterVolumeChanged();
    void statusMessageChanged(const QString &message);
    void analysisChanged();

private:
    AudioEngine *m_engine = nullptr;
};
