#pragma once

#include <QObject>
#include <QString>

// QML-facing track contract. Concrete type is TrackViewModel.
class ITrackViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(float pan READ pan WRITE setPan NOTIFY panChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(bool solo READ solo WRITE setSolo NOTIFY soloChanged)
    Q_PROPERTY(bool audible READ audible NOTIFY audibleChanged)
    Q_PROPERTY(QString volumeText READ volumeText NOTIFY volumeChanged)
    Q_PROPERTY(QString panText READ panText NOTIFY panChanged)
    Q_PROPERTY(float meterLevel READ meterLevel NOTIFY meterLevelChanged)
    Q_PROPERTY(QString meterText READ meterText NOTIFY meterLevelChanged)

public:
    explicit ITrackViewModel(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    virtual QString name() const = 0;
    virtual float volume() const = 0;
    virtual float pan() const = 0;
    virtual bool muted() const = 0;
    virtual bool solo() const = 0;
    virtual bool audible() const = 0;
    virtual QString volumeText() const = 0;
    virtual QString panText() const = 0;
    virtual float meterLevel() const = 0;
    virtual QString meterText() const = 0;

public slots:
    virtual void setVolume(float volume) = 0;
    virtual void setPan(float pan) = 0;
    virtual void setMuted(bool muted) = 0;
    virtual void setSolo(bool solo) = 0;

signals:
    void volumeChanged();
    void panChanged();
    void mutedChanged();
    void soloChanged();
    void audibleChanged();
    void meterLevelChanged();
};
