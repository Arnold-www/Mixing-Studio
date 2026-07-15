#pragma once

#include <QObject>
#include <QQmlListProperty>
#include <QString>
#include <QStringList>
#include <QVariantList>

class ITrackViewModel;

// QML-facing mixer contract. App injects IMixerViewModel*; QML must not know RealMixerViewModel.
class IMixerViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<ITrackViewModel> tracks READ tracks NOTIFY tracksChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(float masterVolume READ masterVolume WRITE setMasterVolume NOTIFY masterVolumeChanged)
    Q_PROPERTY(int positionSeconds READ positionSeconds NOTIFY playbackPositionChanged)
    Q_PROPERTY(int durationSeconds READ durationSeconds NOTIFY durationChanged)
    Q_PROPERTY(float playbackProgress READ playbackProgress NOTIFY playbackPositionChanged)
    Q_PROPERTY(QString playbackTimeText READ playbackTimeText NOTIFY playbackPositionChanged)
    Q_PROPERTY(bool anySolo READ anySolo NOTIFY soloStateChanged)
    Q_PROPERTY(QVariantList waveformPoints READ waveformPoints NOTIFY waveformPointsChanged)
    Q_PROPERTY(QVariantList spectrumLevels READ spectrumLevels NOTIFY spectrumLevelsChanged)
    Q_PROPERTY(float vuLevel READ vuLevel NOTIFY analysisMetersChanged)
    Q_PROPERTY(float peakLevel READ peakLevel NOTIFY analysisMetersChanged)
    Q_PROPERTY(bool clippingDetected READ clippingDetected NOTIFY analysisMetersChanged)
    Q_PROPERTY(QString assetSearchText READ assetSearchText WRITE setAssetSearchText NOTIFY assetSearchTextChanged)
    Q_PROPERTY(QStringList filteredAssetNames READ filteredAssetNames NOTIFY filteredAssetNamesChanged)
    Q_PROPERTY(QStringList recentProjectNames READ recentProjectNames NOTIFY recentProjectNamesChanged)

public:
    explicit IMixerViewModel(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    virtual QQmlListProperty<ITrackViewModel> tracks() = 0;
    virtual bool playing() const = 0;
    virtual QString statusMessage() const = 0;
    virtual float masterVolume() const = 0;
    virtual int positionSeconds() const = 0;
    virtual int durationSeconds() const = 0;
    virtual float playbackProgress() const = 0;
    virtual QString playbackTimeText() const = 0;
    virtual bool anySolo() const = 0;
    virtual QVariantList waveformPoints() const = 0;
    virtual QVariantList spectrumLevels() const = 0;
    virtual float vuLevel() const = 0;
    virtual float peakLevel() const = 0;
    virtual bool clippingDetected() const = 0;
    virtual QString assetSearchText() const = 0;
    virtual QStringList filteredAssetNames() const = 0;
    virtual QStringList recentProjectNames() const = 0;

public slots:
    virtual void importMockTrack() = 0;
    virtual void importAssetByName(const QString &name) = 0;
    virtual void restoreRecentProject(const QString &name) = 0;
    virtual void saveProject() = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void setMasterVolume(float volume) = 0;
    virtual void seekToProgress(float progress) = 0;
    virtual void setAssetSearchText(const QString &text) = 0;

signals:
    void tracksChanged();
    void playingChanged();
    void statusMessageChanged();
    void masterVolumeChanged();
    void playbackPositionChanged();
    void durationChanged();
    void soloStateChanged();
    void waveformPointsChanged();
    void spectrumLevelsChanged();
    void analysisMetersChanged();
    void assetSearchTextChanged();
    void filteredAssetNamesChanged();
    void recentProjectNamesChanged();
};
