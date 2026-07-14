#pragma once

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVector>

#include <DSP/DspProcessor.h>

struct AudioTrack
{
    QString sourcePath;
    QString displayName;
    float volume = 0.8f;
    float pan = 0.0f;
    bool muted = false;
    bool solo = false;
    float eqLowDb = 0.0f;
    float eqMidDb = 0.0f;
    float eqHighDb = 0.0f;
    float compThreshold = 0.7f;
    float compRatio = 4.0f;
    bool fxBypass = false;
};

class AudioEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool playing READ isPlaying NOTIFY playbackStateChanged)
    Q_PROPERTY(int positionMs READ positionMs NOTIFY positionChanged)
    Q_PROPERTY(int durationMs READ durationMs NOTIFY durationChanged)
    Q_PROPERTY(float masterVolume READ masterVolume WRITE setMasterVolume NOTIFY masterVolumeChanged)
    Q_PROPERTY(int loopStartMs READ loopStartMs NOTIFY loopRangeChanged)
    Q_PROPERTY(int loopEndMs READ loopEndMs NOTIFY loopRangeChanged)

public:
    explicit AudioEngine(QObject *parent = nullptr);

    QStringList tracks() const;
    QList<AudioTrack> trackData() const;
    int trackCount() const;
    bool isPlaying() const;
    int positionMs() const;
    int durationMs() const;
    float masterVolume() const;
    int loopStartMs() const;
    int loopEndMs() const;
    bool trackAudible(int index) const;

    // Offline mix of one mono sample per track through volume/pan/EQ/comp/bypass + master limit.
    StereoSample renderMixFrame(const QVector<float> &trackMonoSamples) const;

public slots:
    void importTrack(const QString &path);
    void clearTracks();
    void play();
    void pause();
    void stop();
    void seek(int positionMs);
    void setMasterVolume(float volume);
    void setLoopRange(int startMs, int endMs);
    void setTrackVolume(int index, float volume);
    void setTrackPan(int index, float pan);
    void setTrackMuted(int index, bool muted);
    void setTrackSolo(int index, bool solo);
    void setTrackEq(int index, float lowDb, float midDb, float highDb);
    void setTrackCompressor(int index, float threshold, float ratio);
    void setTrackFxBypass(int index, bool bypass);

signals:
    void tracksChanged();
    void playbackStateChanged();
    void positionChanged();
    void durationChanged();
    void masterVolumeChanged();
    void loopRangeChanged();
    void trackParamsChanged(int index);
    void statusMessageChanged(const QString &message);

private slots:
    void advancePlayback();

private:
    bool isValidTrackIndex(int index) const;
    void setPlaying(bool playing);
    void setPositionMs(int positionMs);
    void setDurationMs(int durationMs);
    bool anySolo() const;

    static constexpr int kPlaybackTickMs = 50;
    static constexpr int kPlaceholderDurationMs = 180000;

    QTimer m_playbackTimer;
    QList<AudioTrack> m_tracks;
    bool m_isPlaying = false;
    int m_positionMs = 0;
    int m_durationMs = 0;
    int m_loopStartMs = 0;
    int m_loopEndMs = 0;
    float m_masterVolume = 1.0f;
};
