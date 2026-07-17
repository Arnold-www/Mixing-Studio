#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVector>

// Per-track ViewModel. Bound indirectly via MixingStudioViewBinder -> root.tracks.
class TrackViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(float pan READ pan WRITE setPan NOTIFY panChanged)
    Q_PROPERTY(float playbackRate READ playbackRate WRITE setPlaybackRate NOTIFY playbackRateChanged)
    Q_PROPERTY(QString playbackRateText READ playbackRateText NOTIFY playbackRateChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(bool solo READ solo WRITE setSolo NOTIFY soloChanged)
    Q_PROPERTY(bool audible READ audible NOTIFY audibleChanged)
    Q_PROPERTY(QString volumeText READ volumeText NOTIFY volumeChanged)
    Q_PROPERTY(QString panText READ panText NOTIFY panChanged)
    Q_PROPERTY(float meterLevel READ meterLevel NOTIFY meterLevelChanged)
    Q_PROPERTY(QString meterText READ meterText NOTIFY meterLevelChanged)
    Q_PROPERTY(float eqLowDb READ eqLowDb WRITE setEqLowDb NOTIFY eqChanged)
    Q_PROPERTY(float eqMidDb READ eqMidDb WRITE setEqMidDb NOTIFY eqChanged)
    Q_PROPERTY(float eqHighDb READ eqHighDb WRITE setEqHighDb NOTIFY eqChanged)
    Q_PROPERTY(QVariantList eqBandDb READ eqBandDb NOTIFY eqChanged)
    Q_PROPERTY(float compThreshold READ compThreshold WRITE setCompThreshold NOTIFY compressorChanged)
    Q_PROPERTY(float compRatio READ compRatio WRITE setCompRatio NOTIFY compressorChanged)
    Q_PROPERTY(bool fxBypass READ fxBypass WRITE setFxBypass NOTIFY fxBypassChanged)
    Q_PROPERTY(bool loopEnabled READ loopEnabled WRITE setLoopEnabled NOTIFY loopChanged)
    Q_PROPERTY(float loopStartProgress READ loopStartProgress NOTIFY loopChanged)
    Q_PROPERTY(float loopEndProgress READ loopEndProgress NOTIFY loopChanged)

public:
    static constexpr int kEqBandCount = 10;

    explicit TrackViewModel(QString name, QObject *parent = nullptr);

    QString name() const;
    float volume() const;
    float pan() const;
    float playbackRate() const;
    QString playbackRateText() const;
    bool muted() const;
    bool solo() const;
    bool audible() const;
    QString volumeText() const;
    QString panText() const;
    float meterLevel() const;
    QString meterText() const;
    float eqLowDb() const;
    float eqMidDb() const;
    float eqHighDb() const;
    QVariantList eqBandDb() const;
    float compThreshold() const;
    float compRatio() const;
    bool fxBypass() const;
    bool loopEnabled() const;
    float loopStartProgress() const;
    float loopEndProgress() const;
    QVector<float> eqBands() const;

    void setBlockedBySolo(bool blockedBySolo);
    void setMeterLevel(float meterLevel);
    void applyLoadedDsp(float volume,
                        float pan,
                        bool muted,
                        bool solo,
                        float eqLowDb,
                        float eqMidDb,
                        float eqHighDb,
                        float compThreshold,
                        float compRatio,
                        bool fxBypass,
                        const QVector<float> &eqBands = {},
                        float playbackRate = 1.0f,
                        bool loopEnabled = false,
                        float loopStart = 0.0f,
                        float loopEnd = 1.0f);

public slots:
    void setVolume(float volume);
    void setPan(float pan);
    void setPlaybackRate(float rate);
    void setMuted(bool muted);
    void setSolo(bool solo);
    void setEqLowDb(float value);
    void setEqMidDb(float value);
    void setEqHighDb(float value);
    void setEqBandDb(int band, float value);
    void setCompThreshold(float value);
    void setCompRatio(float value);
    void setFxBypass(bool bypass);
    void setLoopEnabled(bool enabled);
    void setLoopRangeByProgress(float startProgress, float endProgress);

signals:
    void volumeChanged();
    void panChanged();
    void playbackRateChanged();
    void mutedChanged();
    void soloChanged();
    void audibleChanged();
    void meterLevelChanged();
    void eqChanged();
    void compressorChanged();
    void fxBypassChanged();
    void loopChanged();
    void dspParamsChanged();

private:
    void ensureEqBands();
    void syncLegacyFromBands();
    void syncBandsFromLegacy();

    QString m_name;
    float m_volume = 0.8f;
    float m_pan = 0.0f;
    float m_playbackRate = 1.0f;
    bool m_muted = false;
    bool m_solo = false;
    bool m_blockedBySolo = false;
    float m_meterLevel = 0.0f;
    float m_eqLowDb = 0.0f;
    float m_eqMidDb = 0.0f;
    float m_eqHighDb = 0.0f;
    QVector<float> m_eqBands;
    float m_compThreshold = 0.7f;
    float m_compRatio = 4.0f;
    bool m_fxBypass = false;
    bool m_loopEnabled = false;
    float m_loopStart = 0.0f;
    float m_loopEnd = 1.0f;
};
