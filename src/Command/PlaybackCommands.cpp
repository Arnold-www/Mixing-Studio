#include <Command/PlaybackCommands.h>

#include <Model/AudioEngine.h>

#include <algorithm>

PlayCommand::PlayCommand(AudioEngine *engine)
    : m_engine(engine)
{
}

void PlayCommand::execute()
{
    if (m_engine) {
        m_engine->play();
    }
}

PauseCommand::PauseCommand(AudioEngine *engine)
    : m_engine(engine)
{
}

void PauseCommand::execute()
{
    if (m_engine) {
        m_engine->pause();
    }
}

StopCommand::StopCommand(AudioEngine *engine)
    : m_engine(engine)
{
}

void StopCommand::execute()
{
    if (m_engine) {
        m_engine->stop();
    }
}

SeekProgressCommand::SeekProgressCommand(AudioEngine *engine, float progress)
    : m_engine(engine)
    , m_progress(progress)
{
}

void SeekProgressCommand::execute()
{
    if (!m_engine || m_engine->durationMs() <= 0) {
        return;
    }

    const float clamped = std::clamp(m_progress, 0.0f, 1.0f);
    m_engine->seek(static_cast<int>(clamped * m_engine->durationMs()));
}

SetMasterVolumeCommand::SetMasterVolumeCommand(AudioEngine *engine, float volume)
    : m_engine(engine)
    , m_volume(volume)
{
}

void SetMasterVolumeCommand::execute()
{
    m_clamped = std::clamp(m_volume, 0.0f, 1.0f);
    if (m_engine) {
        m_engine->setMasterVolume(m_clamped);
    }
}

float SetMasterVolumeCommand::clampedVolume() const
{
    return m_clamped;
}
