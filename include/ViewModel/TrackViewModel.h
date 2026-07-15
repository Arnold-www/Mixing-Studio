#pragma once

#include <ViewModel/ITrackViewModel.h>

#include <QString>

class TrackViewModel : public ITrackViewModel
{
    Q_OBJECT

public:
    explicit TrackViewModel(QString name, QObject *parent = nullptr);

    QString name() const override;
    float volume() const override;
    float pan() const override;
    bool muted() const override;
    bool solo() const override;
    bool audible() const override;
    QString volumeText() const override;
    QString panText() const override;
    float meterLevel() const override;
    QString meterText() const override;

    void setBlockedBySolo(bool blockedBySolo);
    void setMeterLevel(float meterLevel);

public slots:
    void setVolume(float volume) override;
    void setPan(float pan) override;
    void setMuted(bool muted) override;
    void setSolo(bool solo) override;

signals:
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
