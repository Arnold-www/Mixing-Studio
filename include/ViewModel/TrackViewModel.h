#pragma once

#include <ViewModel/ITrackViewModel.h>

#include <QString>
#include <QVariantList>
#include <QVector>

class TrackViewModel : public ITrackViewModel
{
    Q_OBJECT

public:
    static constexpr int kEqBandCount = 10;

    explicit TrackViewModel(QString name, QObject *parent = nullptr);

    QString name() const override;
    float volume() const override;
    float pan() const override;
    float playbackRate() const override;
    QString playbackRateText() const override;
    bool muted() const override;
    bool solo() const override;
    bool audible() const override;
    QString volumeText() const override;
    QString panText() const override;
    float meterLevel() const override;
    QString meterText() const override;
    float eqLowDb() const override;
    float eqMidDb() const override;
    float eqHighDb() const override;
    QVariantList eqBandDb() const override;
    float compThreshold() const override;
    float compRatio() const override;
    bool fxBypass() const override;
    bool loopEnabled() const override;
    float loopStartProgress() const override;
    float loopEndProgress() const override;
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
    void setVolume(float volume) override;
    void setPan(float pan) override;
    void setPlaybackRate(float rate) override;
    void setMuted(bool muted) override;
    void setSolo(bool solo) override;
    void setEqLowDb(float value) override;
    void setEqMidDb(float value) override;
    void setEqHighDb(float value) override;
    void setEqBandDb(int band, float value) override;
    void setCompThreshold(float value) override;
    void setCompRatio(float value) override;
    void setFxBypass(bool bypass) override;
    void setLoopEnabled(bool enabled) override;
    void setLoopRangeByProgress(float startProgress, float endProgress) override;

signals:
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
