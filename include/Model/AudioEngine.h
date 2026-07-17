#pragma once

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QAudio>
#include <QElapsedTimer>
#include <QIODevice>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVector>

#include <DSP/DspProcessor.h>
#include <Model/AudioTrack.h>

class AssetLibrary;

// Pulls mixed PCM from AudioEngine for QAudioSink.
class MixAudioDevice final : public QIODevice
{
    Q_OBJECT
public:
    explicit MixAudioDevice(class AudioEngine *engine, QObject *parent = nullptr);

    bool isSequential() const override;
    qint64 bytesAvailable() const override;
    qint64 size() const override;

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    AudioEngine *m_engine = nullptr;
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
    qint64 positionUSecs() const; // sub-ms clock for smooth Seek / playhead
    int durationMs() const;
    float masterVolume() const;
    int loopStartMs() const;
    int loopEndMs() const;
    bool trackAudible(int index) const;
    int outputSampleRate() const;

    QVector<float> waveformPoints() const;
    QVector<float> spectrumLevels() const;
    float vuLevel() const;
    float peakLevel() const;
    bool clippingDetected() const;

    StereoSample renderMixFrame(const QVector<float> &trackMonoSamples) const;
    bool contentLoopActive() const; // any track loopEnabled (content only; does not wrap transport)
    StereoSample renderMixAtSeconds(double seconds, bool loopContent) const;
    StereoSample renderMixAtMs(int positionMs, bool loopContent = false) const;
    StereoSample renderMixAtPlayhead() const;
    QVector<float> trackSamplesAtSeconds(double seconds, bool loopContent) const;
    QVector<float> trackSamplesAtMs(int positionMs, bool loopContent = false) const;
    int pullInterleavedPcm(char *data, int maxBytes); // used by MixAudioDevice
    int pullInterleavedInt16(char *data, int maxBytes); // alias for tests / Int16 path

public slots:
    void importTrack(const QString &path);
    bool importWavFile(const QString &path); // alias for importAudioFile
    bool importAudioFile(const QString &path);
    void clearTracks();
    bool removeTrack(int index);
    void play();
    void pause();
    void stop();
    void seek(int positionMs);
    void setMasterVolume(float volume);
    void setLoopRange(int startMs, int endMs);
    void setTrackVolume(int index, float volume);
    void setTrackPan(int index, float pan);
    void setTrackPlaybackRate(int index, float rate);
    void setTrackMuted(int index, bool muted);
    void setTrackSolo(int index, bool solo);
    void setTrackEq(int index, float lowDb, float midDb, float highDb);
    void setTrackEqBands(int index, const QVector<float> &bands);
    void setTrackEqBand(int index, int band, float gainDb);
    void setTrackCompressor(int index, float threshold, float ratio);
    void setTrackFxBypass(int index, bool bypass);
    void setTrackLoopEnabled(int index, bool enabled);
    void setTrackLoopRange(int index, float startProgress, float endProgress);
    void setTrackVolumeAutomation(int index, const QVector<AutomationPoint> &points);
    float trackAutomationVolumeAtMs(int index, int positionMs) const;
    void refreshAnalysis();
    bool saveProject(const QString &path);
    bool loadProject(const QString &path);
    bool exportMixToWav(const QString &path, int durationMs = -1);
    bool openAssetLibrary(const QString &databasePath);
    bool registerAsset(const QString &path, const QString &displayName = QString());
    QStringList searchAssets(const QString &query) const;
    QStringList recentAssets(int limit = 10) const;
    QString resolveAssetPath(const QString &displayName) const;

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
    void onAudioStateChanged(QAudio::State state);
    void onMeterTimer();
    void emitQueuedPositionChanged();

private:
    bool isValidTrackIndex(int index) const;
    void setPlaying(bool playing);
    void setPositionMs(int positionMs);
    void setDurationMs(int durationMs);
    bool anySolo() const;
    void rebuildAnalysisBuffer();
    void rebuildOverviewWaveform();
    void recomputeDuration();
    void ensureAudioOutput();
    void stopAudioOutput();
    void startUiMeterTimer();
    void stopUiMeterTimer();
    void schedulePositionNotify();
    void captureAnalysisSample(float mono);
    int audiblePositionMs() const;
    qint64 audiblePositionUSecs() const;
    qint64 wrapAudibleUSecs(qint64 heardUs) const;
    void resyncSmoothAudibleClock();
    void syncTransportFromAudible();
    AudioTrack makePlaceholderTrack(const QString &name) const;
    QAudioFormat chooseOutputFormat(const QAudioDevice &device) const;

    static constexpr int kPlaybackTickMs = 16; // timer-clock fallback ~60 Hz
    static constexpr int kMeterTickMs = 8; // ~120 Hz Seek / playhead
    static constexpr int kAnalysisEveryNMeters = 2; // VU + spectrum ~60 Hz
    static constexpr int kPositionUiMinDeltaMs = 8;
    static constexpr qint64 kSmoothExtrapolateMaxUs = 40000; // 40ms cap
    static constexpr int kPlaceholderDurationMs = 3000;
    static constexpr int kAnalysisBufferSamples = 512;
    static constexpr int kWaveformBins = 256;
    static constexpr int kSpectrumBands = 32;
    static constexpr int kExportSampleRate = 44100;
    static constexpr int kCaptureDecimate = 2;

    QTimer m_playbackTimer;
    QTimer m_meterTimer;
    QList<AudioTrack> m_tracks;
    bool m_isPlaying = false;
    int m_positionMs = 0;
    int m_durationMs = 0;
    int m_loopStartMs = 0;
    int m_loopEndMs = 0;
    float m_masterVolume = 1.0f;
    int m_outputSampleRate = kExportSampleRate;
    int m_outputChannelCount = 2;
    QAudioFormat::SampleFormat m_outputSampleFormat = QAudioFormat::Int16;

    QVector<float> m_analysisBuffer;
    QVector<float> m_captureRing;
    mutable QMutex m_captureMutex;
    int m_captureWrite = 0;
    int m_captureDecimateCounter = 0;
    int m_meterFrame = 0;
    QVector<float> m_waveformPoints;
    QVector<float> m_spectrumLevels;
    float m_vuLevel = 0.0f;
    float m_peakLevel = 0.0f;
    bool m_clippingDetected = false;
    AssetLibrary *m_assetLibrary = nullptr;

    QAudioSink *m_audioSink = nullptr;
    MixAudioDevice *m_audioDevice = nullptr;
    bool m_useAudioSink = false;
    qint64 m_playheadSamples = 0;
    int m_playStartPositionMs = 0; // timeline origin when sink start()/restart
    qint64 m_lastDeviceProcessedUSecs = -1;
    qint64 m_lastEmittedPositionUSecs = -1;
    int m_lastQueuedPositionMs = -1;
    bool m_positionNotifyPending = false;
    QElapsedTimer m_smoothAudibleClock;
};
