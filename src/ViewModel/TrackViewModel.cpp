#include <ViewModel/TrackViewModel.h>

#include <algorithm>
#include <utility>

TrackViewModel::TrackViewModel(QString name, QObject *parent)
    : QObject(parent)
    , m_name(std::move(name))
{
}

QString TrackViewModel::name() const
{
    return m_name;
}

float TrackViewModel::volume() const
{
    return m_volume;
}

float TrackViewModel::pan() const
{
    return m_pan;
}

bool TrackViewModel::muted() const
{
    return m_muted;
}

bool TrackViewModel::solo() const
{
    return m_solo;
}

void TrackViewModel::setVolume(float volume)
{
    const float clamped = std::clamp(volume, 0.0f, 1.0f);
    if (qFuzzyCompare(m_volume, clamped)) {
        return;
    }

    m_volume = clamped;
    emit volumeChanged();
}

void TrackViewModel::setPan(float pan)
{
    const float clamped = std::clamp(pan, -1.0f, 1.0f);
    if (qFuzzyCompare(m_pan, clamped)) {
        return;
    }

    m_pan = clamped;
    emit panChanged();
}

void TrackViewModel::setMuted(bool muted)
{
    if (m_muted == muted) {
        return;
    }

    m_muted = muted;
    emit mutedChanged();
}

void TrackViewModel::setSolo(bool solo)
{
    if (m_solo == solo) {
        return;
    }

    m_solo = solo;
    emit soloChanged();
}
