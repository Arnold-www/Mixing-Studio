#include <ViewModel/MixerViewModel.h>

#include <App/MixerApp.h>
#include <Command/MixerCommands.h>
#include <ViewModel/TrackViewModel.h>

#include <algorithm>
#include <cmath>
#include <utility>

void MixerViewModel::addTrack(const QString &name)
{
    auto *track = new TrackViewModel(name, this);
    const int index = m_tracks.size();
    connect(track, &TrackViewModel::soloChanged, this, &MixerViewModel::refreshSoloState);
    connect(track, &TrackViewModel::dspParamsChanged, this, [this, index]() {
        syncTrackToEngine(index);
    });
    m_tracks.append(track);
    refreshSoloState();
    syncTrackToEngine(index);
    emit tracksChanged();
}

void MixerViewModel::refreshSoloState()
{
    if (!m_app) {
        return;
    }

    QVector<bool> soloFlags;
    soloFlags.reserve(m_tracks.size());
    for (const TrackViewModel *track : std::as_const(m_tracks)) {
        soloFlags.append(track->solo());
    }

    const SoloPlan plan = MixerApp::planSolo(soloFlags);
    const bool previousAnySolo = m_anySolo;
    m_anySolo = plan.anySolo;

    for (int i = 0; i < m_tracks.size(); ++i) {
        m_tracks.at(i)->setBlockedBySolo(plan.blockedBySolo.at(i));
    }

    syncAllTracksToEngine();

    if (previousAnySolo != m_anySolo) {
        emit soloStateChanged();
        setStatusMessage(m_anySolo ? QStringLiteral("Solo monitoring enabled.")
                                   : QStringLiteral("Solo monitoring cleared."));
    }
}

void MixerViewModel::syncTrackToEngine(int index)
{
    if (!m_app || index < 0 || index >= m_tracks.size()) {
        return;
    }

    const TrackViewModel *track = m_tracks.at(index);
    TrackDspParams params;
    params.volume = track->volume();
    params.pan = track->pan();
    params.muted = track->muted();
    params.solo = track->solo();
    ApplyTrackDspCommand(m_app, index, params).execute();
}

void MixerViewModel::syncAllTracksToEngine()
{
    if (!m_app) {
        return;
    }

    QVector<TrackDspParams> params;
    params.reserve(m_tracks.size());
    for (const TrackViewModel *track : std::as_const(m_tracks)) {
        TrackDspParams item;
        item.volume = track->volume();
        item.pan = track->pan();
        item.muted = track->muted();
        item.solo = track->solo();
        params.append(item);
    }
    ApplyAllTrackDspCommand(m_app, std::move(params)).execute();
}

void MixerViewModel::updateMockAnalysisData()
{
    ++m_analysisFrame;

    QVariantList waveform;
    waveform.reserve(64);
    for (int i = 0; i < 64; ++i) {
        const double phase = (m_analysisFrame * 0.18) + (i * 0.34);
        const double envelope = 0.35 + (0.45 * std::sin((i + m_analysisFrame) * 0.07));
        const double value = std::sin(phase) * envelope;
        waveform.append(std::clamp(value, -1.0, 1.0));
    }

    QVariantList spectrum;
    spectrum.reserve(18);
    for (int i = 0; i < 18; ++i) {
        const double phase = (m_analysisFrame * 0.11) + (i * 0.42);
        const double rolloff = 1.0 - (static_cast<double>(i) / 24.0);
        const double value = (0.22 + (0.58 * std::abs(std::sin(phase)))) * rolloff;
        spectrum.append(std::clamp(value, 0.0, 1.0));
    }

    m_waveformPoints = waveform;
    m_spectrumLevels = spectrum;
    emit waveformPointsChanged();
    emit spectrumLevelsChanged();

    for (int i = 0; i < m_tracks.size(); ++i) {
        const double phase = (m_analysisFrame * 0.15) + (i * 0.8);
        const float level = static_cast<float>(
            (0.28 + (0.56 * std::abs(std::sin(phase)))) * m_tracks.at(i)->volume());
        m_tracks.at(i)->setMeterLevel(level);
    }
}

void MixerViewModel::refreshFilteredAssetNames()
{
    if (!m_app) {
        return;
    }

    const QStringList filtered = m_app->filterAssets(m_assetNames, m_assetSearchText);
    if (m_filteredAssetNames == filtered) {
        return;
    }

    m_filteredAssetNames = filtered;
    emit filteredAssetNamesChanged();
}
