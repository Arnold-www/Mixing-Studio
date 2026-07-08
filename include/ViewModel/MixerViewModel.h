#pragma once

#include <QObject>
#include <QQmlListProperty>
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

public:
    explicit MixerViewModel(AudioEngine *audioEngine, QObject *parent = nullptr);

    QQmlListProperty<TrackViewModel> tracks();
    bool playing() const;
    QString statusMessage() const;
    float masterVolume() const;

public slots:
    void importMockTrack();
    void play();
    void pause();
    void stop();
    void setMasterVolume(float volume);

signals:
    void tracksChanged();
    void playingChanged();
    void statusMessageChanged();
    void masterVolumeChanged();

private:
    void addTrack(const QString &name);
    void setStatusMessage(const QString &message);

    static qsizetype trackCount(QQmlListProperty<TrackViewModel> *property);
    static TrackViewModel *trackAt(QQmlListProperty<TrackViewModel> *property, qsizetype index);

    AudioEngine *m_audioEngine = nullptr;
    QVector<TrackViewModel *> m_tracks;
    QString m_statusMessage = QStringLiteral("Ready.");
    float m_masterVolume = 1.0f;
};
