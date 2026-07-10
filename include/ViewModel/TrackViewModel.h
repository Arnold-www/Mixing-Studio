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

    void setBlockedBySolo(bool blockedBySolo);

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

private:
    QString m_name;
    float m_volume = 0.8f;
    float m_pan = 0.0f;
    bool m_muted = false;
    bool m_solo = false;
    bool m_blockedBySolo = false;
};
