#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

// QML-facing track contract. Concrete type is TrackViewModel.
class ITrackViewModel : public QObject
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
    explicit ITrackViewModel(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    virtual QString name() const = 0;
    virtual float volume() const = 0;
    virtual float pan() const = 0;
    virtual float playbackRate() const = 0;
    virtual QString playbackRateText() const = 0;
    virtual bool muted() const = 0;
    virtual bool solo() const = 0;
    virtual bool audible() const = 0;
    virtual QString volumeText() const = 0;
    virtual QString panText() const = 0;
    virtual float meterLevel() const = 0;
    virtual QString meterText() const = 0;
    virtual float eqLowDb() const = 0;
    virtual float eqMidDb() const = 0;
    virtual float eqHighDb() const = 0;
    virtual QVariantList eqBandDb() const = 0;
    virtual float compThreshold() const = 0;
    virtual float compRatio() const = 0;
    virtual bool fxBypass() const = 0;
    virtual bool loopEnabled() const = 0;
    virtual float loopStartProgress() const = 0;
    virtual float loopEndProgress() const = 0;

public slots:
    virtual void setVolume(float volume) = 0;
    virtual void setPan(float pan) = 0;
    virtual void setPlaybackRate(float rate) = 0;
    virtual void setMuted(bool muted) = 0;
    virtual void setSolo(bool solo) = 0;
    virtual void setEqLowDb(float value) = 0;
    virtual void setEqMidDb(float value) = 0;
    virtual void setEqHighDb(float value) = 0;
    virtual void setEqBandDb(int band, float value) = 0;
    virtual void setCompThreshold(float value) = 0;
    virtual void setCompRatio(float value) = 0;
    virtual void setFxBypass(bool bypass) = 0;
    virtual void setLoopEnabled(bool enabled) = 0;
    virtual void setLoopRangeByProgress(float startProgress, float endProgress) = 0;

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
};
