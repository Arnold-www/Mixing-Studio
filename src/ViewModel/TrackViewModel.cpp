#include <ViewModel/TrackViewModel.h>

#include <algorithm>
#include <cmath>
#include <utility>

TrackViewModel::TrackViewModel(QString name, QObject *parent)
    : ITrackViewModel(parent)
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

bool TrackViewModel::audible() const
{
    return !m_muted && !m_blockedBySolo;
}

QString TrackViewModel::volumeText() const
{
    return QStringLiteral("%1%").arg(static_cast<int>(m_volume * 100.0f));
}

QString TrackViewModel::panText() const
{
    if (qFuzzyIsNull(m_pan)) {
        return QStringLiteral("C");
    }

    const QString side = m_pan < 0.0f ? QStringLiteral("L") : QStringLiteral("R");
    return QStringLiteral("%1%2").arg(side).arg(static_cast<int>(std::abs(m_pan) * 100.0f));
}

float TrackViewModel::meterLevel() const
{
    return audible() ? m_meterLevel : 0.0f;
}

QString TrackViewModel::meterText() const
{
    return QStringLiteral("%1%").arg(static_cast<int>(meterLevel() * 100.0f));
}

void TrackViewModel::setBlockedBySolo(bool blockedBySolo)
{
    if (m_blockedBySolo == blockedBySolo) {
        return;
    }

    const bool wasAudible = audible();
    m_blockedBySolo = blockedBySolo;
    if (wasAudible != audible()) {
        emit audibleChanged();
        emit meterLevelChanged();
        emit dspParamsChanged();
    }
}

void TrackViewModel::setMeterLevel(float meterLevel)
{
    const float clamped = std::clamp(meterLevel, 0.0f, 1.0f);
    if (qFuzzyCompare(m_meterLevel, clamped)) {
        return;
    }

    m_meterLevel = clamped;
    emit meterLevelChanged();
}

void TrackViewModel::setVolume(float volume)
{
    const float clamped = std::clamp(volume, 0.0f, 1.0f);
    if (qFuzzyCompare(m_volume, clamped)) {
        return;
    }

    m_volume = clamped;
    emit volumeChanged();
    emit dspParamsChanged();
}

void TrackViewModel::setPan(float pan)
{
    const float clamped = std::clamp(pan, -1.0f, 1.0f);
    if (qFuzzyCompare(m_pan, clamped)) {
        return;
    }

    m_pan = clamped;
    emit panChanged();
    emit dspParamsChanged();
}

void TrackViewModel::setMuted(bool muted)
{
    if (m_muted == muted) {
        return;
    }

    const bool wasAudible = audible();
    m_muted = muted;
    emit mutedChanged();
    emit dspParamsChanged();
    if (wasAudible != audible()) {
        emit audibleChanged();
        emit meterLevelChanged();
    }
}

void TrackViewModel::setSolo(bool solo)
{
    if (m_solo == solo) {
        return;
    }

    m_solo = solo;
    emit soloChanged();
    emit dspParamsChanged();
}
