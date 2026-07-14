#pragma once

#include <QObject>
#include <QString>

class TrackViewModel : public QObject
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
    explicit TrackViewModel(QString name, QObject *parent = nullptr);

    QString name() const;
    float volume() const;
    float pan() const;
    bool muted() const;
    bool solo() const;
    bool audible() const;
    QString volumeText() const;
    QString panText() const;
    float meterLevel() const;
    QString meterText() const;

    void setBlockedBySolo(bool blockedBySolo);
    void setMeterLevel(float meterLevel);

public slots:
    void setVolume(float volume);
    void setPan(float pan);
    void setMuted(bool muted);
    void setSolo(bool solo);

signals:
    void volumeChanged();
    void panChanged();
    void mutedChanged();
    void soloChanged();
    void audibleChanged();
    void meterLevelChanged();
    void dspParamsChanged();

private:
    QString m_name;
    float m_volume = 0.8f;
    float m_pan = 0.0f;
    bool m_muted = false;
    bool m_solo = false;
    bool m_blockedBySolo = false;
    float m_meterLevel = 0.0f;
};
