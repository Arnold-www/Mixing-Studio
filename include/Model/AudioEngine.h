#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class AudioEngine : public QObject
{
    Q_OBJECT

public:
    explicit AudioEngine(QObject *parent = nullptr);

    QStringList tracks() const;
    bool isPlaying() const;
    float masterVolume() const;

public slots:
    void importTrack(const QString &path);
    void play();
    void pause();
    void stop();
    void setMasterVolume(float volume);

signals:
    void tracksChanged();
    void playbackStateChanged();
    void statusMessageChanged(const QString &message);

private:
    QStringList m_tracks;
    bool m_isPlaying = false;
    float m_masterVolume = 1.0f;
};
