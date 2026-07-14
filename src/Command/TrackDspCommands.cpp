#include <Command/TrackDspCommands.h>

#include <utility>

ApplyTrackDspCommand::ApplyTrackDspCommand(MixerApp *app, int index, TrackDspParams params)
    : m_app(app)
    , m_index(index)
    , m_params(std::move(params))
{
}

void ApplyTrackDspCommand::execute()
{
    if (m_app) {
        m_app->applyTrackDsp(m_index, m_params);
    }
}

ApplyAllTrackDspCommand::ApplyAllTrackDspCommand(MixerApp *app, QVector<TrackDspParams> params)
    : m_app(app)
    , m_params(std::move(params))
{
}

void ApplyAllTrackDspCommand::execute()
{
    if (m_app) {
        m_app->applyAllTrackDsp(m_params);
    }
}
