#pragma once

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVector>

#include <DSP/DspProcessor.h>
#include <Model/AudioTrack.h>

class AssetLibrary;

class AudioEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool playing READ isPlaying NOTIFY playbackStateChanged)
    Q_PROPERTY(int positionMs READ positionMs NOTIFY positionChanged)
    Q_PROPERTY(int durationMs READ durationMs NOTIFY durationChanged)
    Q_PROPERTY(float masterVolume READ masterVolume WRITE setMasterVolume NOTIFY masterVolumeChanged)
    Q_PROPERTY(int loopStartMs READ loopStartMs NOTIFY loopRangeChanged)
    Q_PROPERTY(int loopEndMs READ loopEndMs NOTIFY loopRangeChanged)
    Q_PROPERTY(QVector<float> waveformPoints READ waveformPoints NOTIFY analysisChanged)
    Q_PROPERTY(QVector<float> spectrumLevels READ spectrumLevels NOTIFY analysisChanged)
    Q_PROPERTY(float vuLevel READ vuLevel NOTIFY analysisChanged)
    Q_PROPERTY(float peakLevel READ peakLevel NOTIFY analysisChanged)
    Q_PROPERTY(bool clippingDetected READ clippingDetected NOTIFY analysisChanged)

public:
    explicit AudioEngine(QObject *parent = nullptr);
    ~AudioEngine() override;

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

    QVector<float> waveformPoints() const;
    QVector<float> spectrumLevels() const;
    float vuLevel() const;
    float peakLevel() const;
    bool clippingDetected() const;

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
    void refreshAnalysis();
    bool saveProject(const QString &path);
    bool loadProject(const QString &path);
    bool openAssetLibrary(const QString &databasePath);
    bool registerAsset(const QString &path, const QString &displayName = QString());
    QStringList searchAssets(const QString &query) const;
    QStringList recentAssets(int limit = 10) const;

signals:
    void tracksChanged();
    void playbackStateChanged();
    void positionChanged();
    void durationChanged();
    void masterVolumeChanged();
    void loopRangeChanged();
    void trackParamsChanged(int index);
    void analysisChanged();
    void statusMessageChanged(const QString &message);

private slots:
    void advancePlayback();

private:
    bool isValidTrackIndex(int index) const;
    void setPlaying(bool playing);
    void setPositionMs(int positionMs);
    void setDurationMs(int durationMs);
    bool anySolo() const;
    void rebuildPlaceholderBuffer();

    static constexpr int kPlaybackTickMs = 50;
    static constexpr int kPlaceholderDurationMs = 180000;
    static constexpr int kAnalysisBufferSamples = 512;
    static constexpr int kWaveformBins = 64;
    static constexpr int kSpectrumBands = 18;

    QTimer m_playbackTimer;
    QList<AudioTrack> m_tracks;
    bool m_isPlaying = false;
    int m_positionMs = 0;
    int m_durationMs = 0;
    int m_loopStartMs = 0;
    int m_loopEndMs = 0;
    float m_masterVolume = 1.0f;

    QVector<float> m_analysisBuffer;
    QVector<float> m_waveformPoints;
    QVector<float> m_spectrumLevels;
    float m_vuLevel = 0.0f;
    float m_peakLevel = 0.0f;
    bool m_clippingDetected = false;
    AssetLibrary *m_assetLibrary = nullptr;
};
