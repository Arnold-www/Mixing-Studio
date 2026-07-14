#include <Command/PlaybackCommands.h>

PlayCommand::PlayCommand(MixerApp *app)
    : m_app(app)
{
}

void PlayCommand::execute()
{
    if (m_app) {
        m_app->play();
    }
}

PauseCommand::PauseCommand(MixerApp *app)
    : m_app(app)
{
}

void PauseCommand::execute()
{
    if (m_app) {
        m_app->pause();
    }
}

StopCommand::StopCommand(MixerApp *app)
    : m_app(app)
{
}

void StopCommand::execute()
{
    if (m_app) {
        m_app->stop();
    }
}

SeekProgressCommand::SeekProgressCommand(MixerApp *app, float progress)
    : m_app(app)
    , m_progress(progress)
{
}

void SeekProgressCommand::execute()
{
    if (m_app) {
        m_app->seekToProgress(m_progress);
    }
}

SetMasterVolumeCommand::SetMasterVolumeCommand(MixerApp *app, float volume)
    : m_app(app)
    , m_volume(volume)
{
}

void SetMasterVolumeCommand::execute()
{
    if (m_app) {
        m_clamped = m_app->setMasterVolume(m_volume);
    }
}

float SetMasterVolumeCommand::clampedVolume() const
{
    return m_clamped;
}
