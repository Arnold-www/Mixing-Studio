#include <ViewModel/MixerViewModel.h>

#include <Model/AudioEngine.h>
#include <ViewModel/TrackViewModel.h>

#include <algorithm>

MixerViewModel::MixerViewModel(AudioEngine *audioEngine, QObject *parent)
    : QObject(parent)
    , m_audioEngine(audioEngine)
{
    connect(m_audioEngine, &AudioEngine::playbackStateChanged, this, &MixerViewModel::playingChanged);
    connect(m_audioEngine, &AudioEngine::statusMessageChanged, this, &MixerViewModel::setStatusMessage);
}

QQmlListProperty<TrackViewModel> MixerViewModel::tracks()
{
    return QQmlListProperty<TrackViewModel>(this, &m_tracks, &MixerViewModel::trackCount, &MixerViewModel::trackAt);
}

bool MixerViewModel::playing() const
{
    return m_audioEngine->isPlaying();
}

QString MixerViewModel::statusMessage() const
{
    return m_statusMessage;
}

float MixerViewModel::masterVolume() const
{
    return m_masterVolume;
}

void MixerViewModel::importMockTrack()
{
    const QString name = QStringLiteral("Track %1").arg(m_tracks.size() + 1);
    m_audioEngine->importTrack(name);
    addTrack(name);
}

void MixerViewModel::play()
{
    m_audioEngine->play();
}

void MixerViewModel::pause()
{
    m_audioEngine->pause();
}

void MixerViewModel::stop()
{
    m_audioEngine->stop();
}

void MixerViewModel::setMasterVolume(float volume)
{
    const float clamped = std::clamp(volume, 0.0f, 1.0f);
    if (qFuzzyCompare(m_masterVolume, clamped)) {
        return;
    }

    m_masterVolume = clamped;
    m_audioEngine->setMasterVolume(m_masterVolume);
    emit masterVolumeChanged();
}

void MixerViewModel::addTrack(const QString &name)
{
    m_tracks.append(new TrackViewModel(name, this));
    emit tracksChanged();
}

void MixerViewModel::setStatusMessage(const QString &message)
{
    if (m_statusMessage == message) {
        return;
    }

    m_statusMessage = message;
    emit statusMessageChanged();
}

qsizetype MixerViewModel::trackCount(QQmlListProperty<TrackViewModel> *property)
{
    const auto tracks = static_cast<QVector<TrackViewModel *> *>(property->data);
    return tracks->size();
}

TrackViewModel *MixerViewModel::trackAt(QQmlListProperty<TrackViewModel> *property, qsizetype index)
{
    const auto tracks = static_cast<QVector<TrackViewModel *> *>(property->data);
    return tracks->at(index);
}
