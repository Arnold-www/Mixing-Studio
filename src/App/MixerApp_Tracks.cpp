#include <App/MixerApp.h>

#include <Model/AudioEngine.h>

#include <algorithm>

SoloPlan MixerApp::planSolo(const QVector<bool> &soloFlags)
{
    SoloPlan plan;
    plan.anySolo = std::any_of(soloFlags.cbegin(), soloFlags.cend(), [](bool solo) {
        return solo;
    });
    plan.blockedBySolo.reserve(soloFlags.size());
    for (bool solo : soloFlags) {
        plan.blockedBySolo.append(plan.anySolo && !solo);
    }
    return plan;
}

void MixerApp::applyTrackDsp(int index, const TrackDspParams &params)
{
    if (!m_engine) {
        return;
    }

    m_engine->setTrackVolume(index, params.volume);
    m_engine->setTrackPan(index, params.pan);
    m_engine->setTrackMuted(index, params.muted);
    m_engine->setTrackSolo(index, params.solo);
}

void MixerApp::applyAllTrackDsp(const QVector<TrackDspParams> &params)
{
    for (int i = 0; i < params.size(); ++i) {
        applyTrackDsp(i, params.at(i));
    }
}
