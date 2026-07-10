#pragma once

#include <QObject>
#include <QQmlListProperty>
#include <QTimer>
#include <QVector>

class AudioEngine;
class TrackViewModel;

class MixerViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<TrackViewModel> tracks READ tracks NOTIFY tracksChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(float masterVolume READ masterVolume WRITE setMasterVolume NOTIFY masterVolumeChanged)
    Q_PROPERTY(int positionSeconds READ positionSeconds NOTIFY playbackPositionChanged)
    Q_PROPERTY(int durationSeconds READ durationSeconds NOTIFY durationChanged)
    Q_PROPERTY(float playbackProgress READ playbackProgress NOTIFY playbackPositionChanged)
    Q_PROPERTY(QString playbackTimeText READ playbackTimeText NOTIFY playbackPositionChanged)
    Q_PROPERTY(bool anySolo READ anySolo NOTIFY soloStateChanged)

public:
    explicit MixerViewModel(AudioEngine *audioEngine, QObject *parent = nullptr);

    QQmlListProperty<TrackViewModel> tracks();
    bool playing() const;
    QString statusMessage() const;
    float masterVolume() const;
    int positionSeconds() const;
    int durationSeconds() const;
    float playbackProgress() const;
    QString playbackTimeText() const;
    bool anySolo() const;

public slots:
    void importMockTrack();
    void play();
    void pause();
    void stop();
    void setMasterVolume(float volume);
    void seekToProgress(float progress);

signals:
    void tracksChanged();
    void playingChanged();
    void statusMessageChanged();
    void masterVolumeChanged();
    void playbackPositionChanged();
    void durationChanged();
    void soloStateChanged();

private:
    void addTrack(const QString &name);
    void setStatusMessage(const QString &message);
    void setPositionSeconds(int positionSeconds);
    void updatePlaybackTimer();
    void refreshSoloState();
    QString formatTime(int seconds) const;

    static qsizetype trackCount(QQmlListProperty<TrackViewModel> *property);
    static TrackViewModel *trackAt(QQmlListProperty<TrackViewModel> *property, qsizetype index);

    AudioEngine *m_audioEngine = nullptr;
    QVector<TrackViewModel *> m_tracks;
    QString m_statusMessage = QStringLiteral("Ready.");
    float m_masterVolume = 1.0f;
    int m_positionSeconds = 0;
    int m_durationSeconds = 180;
    bool m_anySolo = false;
    QTimer m_playbackTimer;
};
