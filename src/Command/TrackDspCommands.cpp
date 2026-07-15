#include <Command/TrackDspCommands.h>

#include <Model/AudioEngine.h>

#include <utility>

ApplyTrackDspCommand::ApplyTrackDspCommand(AudioEngine *engine, int index, TrackDspParams params)
    : m_engine(engine)
    , m_index(index)
    , m_params(std::move(params))
{
}

void ApplyTrackDspCommand::execute()
{
    if (!m_engine) {
        return;
    }

    m_engine->setTrackVolume(m_index, m_params.volume);
    m_engine->setTrackPan(m_index, m_params.pan);
    m_engine->setTrackMuted(m_index, m_params.muted);
    m_engine->setTrackSolo(m_index, m_params.solo);
}

ApplyAllTrackDspCommand::ApplyAllTrackDspCommand(AudioEngine *engine, QVector<TrackDspParams> params)
    : m_engine(engine)
    , m_params(std::move(params))
{
}

void ApplyAllTrackDspCommand::execute()
{
    for (int i = 0; i < m_params.size(); ++i) {
        ApplyTrackDspCommand(m_engine, i, m_params.at(i)).execute();
    }
}
