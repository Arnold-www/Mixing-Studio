#include <ViewModel/TrackViewModel.h>

#include <algorithm>
#include <cmath>
#include <utility>

TrackViewModel::TrackViewModel(QString name, QObject *parent)
    : ITrackViewModel(parent)
    , m_name(std::move(name))
{
    ensureEqBands();
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

float TrackViewModel::playbackRate() const
{
    return m_playbackRate;
}

QString TrackViewModel::playbackRateText() const
{
    return QStringLiteral("%1x").arg(m_playbackRate, 0, 'f', 2);
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
    // ~0.5% steps so the level bar / % text track audio smoothly.
    if (std::fabs(m_meterLevel - clamped) < 0.005f) {
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

void TrackViewModel::setPlaybackRate(float rate)
{
    const float clamped = std::clamp(rate, 0.5f, 2.0f);
    if (qFuzzyCompare(m_playbackRate, clamped)) {
        return;
    }

    m_playbackRate = clamped;
    emit playbackRateChanged();
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

float TrackViewModel::eqLowDb() const { return m_eqLowDb; }
float TrackViewModel::eqMidDb() const { return m_eqMidDb; }
float TrackViewModel::eqHighDb() const { return m_eqHighDb; }

QVariantList TrackViewModel::eqBandDb() const
{
    QVariantList list;
    list.reserve(m_eqBands.size());
    for (float value : m_eqBands) {
        list.append(value);
    }
    return list;
}

QVector<float> TrackViewModel::eqBands() const
{
    return m_eqBands;
}

float TrackViewModel::compThreshold() const { return m_compThreshold; }
float TrackViewModel::compRatio() const { return m_compRatio; }
bool TrackViewModel::fxBypass() const { return m_fxBypass; }
bool TrackViewModel::loopEnabled() const { return m_loopEnabled; }
float TrackViewModel::loopStartProgress() const { return m_loopStart; }
float TrackViewModel::loopEndProgress() const { return m_loopEnd; }

void TrackViewModel::ensureEqBands()
{
    if (m_eqBands.size() == kEqBandCount) {
        return;
    }
    m_eqBands = QVector<float>(kEqBandCount, 0.0f);
    m_eqBands[1] = m_eqLowDb;
    m_eqBands[4] = m_eqMidDb;
    m_eqBands[7] = m_eqHighDb;
}

void TrackViewModel::syncLegacyFromBands()
{
    ensureEqBands();
    m_eqLowDb = m_eqBands[1];
    m_eqMidDb = m_eqBands[4];
    m_eqHighDb = m_eqBands[7];
}

void TrackViewModel::syncBandsFromLegacy()
{
    ensureEqBands();
    m_eqBands[1] = m_eqLowDb;
    m_eqBands[4] = m_eqMidDb;
    m_eqBands[7] = m_eqHighDb;
}

void TrackViewModel::setEqLowDb(float value)
{
    const float clamped = std::clamp(value, -12.0f, 12.0f);
    if (qFuzzyCompare(m_eqLowDb, clamped)) {
        return;
    }
    m_eqLowDb = clamped;
    syncBandsFromLegacy();
    emit eqChanged();
    emit dspParamsChanged();
}

void TrackViewModel::setEqMidDb(float value)
{
    const float clamped = std::clamp(value, -12.0f, 12.0f);
    if (qFuzzyCompare(m_eqMidDb, clamped)) {
        return;
    }
    m_eqMidDb = clamped;
    syncBandsFromLegacy();
    emit eqChanged();
    emit dspParamsChanged();
}

void TrackViewModel::setEqHighDb(float value)
{
    const float clamped = std::clamp(value, -12.0f, 12.0f);
    if (qFuzzyCompare(m_eqHighDb, clamped)) {
        return;
    }
    m_eqHighDb = clamped;
    syncBandsFromLegacy();
    emit eqChanged();
    emit dspParamsChanged();
}

void TrackViewModel::setEqBandDb(int band, float value)
{
    if (band < 0 || band >= kEqBandCount) {
        return;
    }
    ensureEqBands();
    const float clamped = std::clamp(value, -12.0f, 12.0f);
    if (qFuzzyCompare(m_eqBands[band], clamped)) {
        return;
    }
    m_eqBands[band] = clamped;
    syncLegacyFromBands();
    emit eqChanged();
    emit dspParamsChanged();
}

void TrackViewModel::setCompThreshold(float value)
{
    const float clamped = std::clamp(value, 0.05f, 1.0f);
    if (qFuzzyCompare(m_compThreshold, clamped)) {
        return;
    }
    m_compThreshold = clamped;
    emit compressorChanged();
    emit dspParamsChanged();
}

void TrackViewModel::setCompRatio(float value)
{
    const float clamped = std::clamp(value, 1.0f, 20.0f);
    if (qFuzzyCompare(m_compRatio, clamped)) {
        return;
    }
    m_compRatio = clamped;
    emit compressorChanged();
    emit dspParamsChanged();
}

void TrackViewModel::setFxBypass(bool bypass)
{
    if (m_fxBypass == bypass) {
        return;
    }
    m_fxBypass = bypass;
    emit fxBypassChanged();
    emit dspParamsChanged();
}

void TrackViewModel::setLoopEnabled(bool enabled)
{
    if (m_loopEnabled == enabled) {
        return;
    }
    m_loopEnabled = enabled;
    emit loopChanged();
    emit dspParamsChanged();
}

void TrackViewModel::setLoopRangeByProgress(float startProgress, float endProgress)
{
    float s = std::clamp(startProgress, 0.0f, 1.0f);
    float e = std::clamp(endProgress, 0.0f, 1.0f);
    if (e < s + 0.02f) {
        e = std::min(1.0f, s + 0.02f);
    }
    if (qFuzzyCompare(m_loopStart, s) && qFuzzyCompare(m_loopEnd, e)) {
        return;
    }
    m_loopStart = s;
    m_loopEnd = e;
    emit loopChanged();
    emit dspParamsChanged();
}

void TrackViewModel::applyLoadedDsp(float volume,
                                    float pan,
                                    bool muted,
                                    bool solo,
                                    float eqLowDb,
                                    float eqMidDb,
                                    float eqHighDb,
                                    float compThreshold,
                                    float compRatio,
                                    bool fxBypass,
                                    const QVector<float> &eqBands,
                                    float playbackRate,
                                    bool loopEnabled,
                                    float loopStart,
                                    float loopEnd)
{
    m_volume = std::clamp(volume, 0.0f, 1.0f);
    m_pan = std::clamp(pan, -1.0f, 1.0f);
    m_playbackRate = std::clamp(playbackRate, 0.5f, 2.0f);
    m_muted = muted;
    m_solo = solo;
    m_eqLowDb = std::clamp(eqLowDb, -12.0f, 12.0f);
    m_eqMidDb = std::clamp(eqMidDb, -12.0f, 12.0f);
    m_eqHighDb = std::clamp(eqHighDb, -12.0f, 12.0f);
    m_compThreshold = std::clamp(compThreshold, 0.05f, 1.0f);
    m_compRatio = std::clamp(compRatio, 1.0f, 20.0f);
    m_fxBypass = fxBypass;
    m_loopEnabled = loopEnabled;
    m_loopStart = std::clamp(loopStart, 0.0f, 1.0f);
    m_loopEnd = std::clamp(loopEnd, 0.0f, 1.0f);
    if (m_loopEnd < m_loopStart + 0.02f) {
        m_loopEnd = std::min(1.0f, m_loopStart + 0.02f);
    }

    m_eqBands = QVector<float>(kEqBandCount, 0.0f);
    if (eqBands.size() >= kEqBandCount) {
        for (int i = 0; i < kEqBandCount; ++i) {
            m_eqBands[i] = std::clamp(eqBands.at(i), -12.0f, 12.0f);
        }
        syncLegacyFromBands();
    } else {
        syncBandsFromLegacy();
    }

    emit volumeChanged();
    emit panChanged();
    emit playbackRateChanged();
    emit mutedChanged();
    emit soloChanged();
    emit eqChanged();
    emit compressorChanged();
    emit fxBypassChanged();
    emit loopChanged();
    emit audibleChanged();
}

