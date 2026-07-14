#include <App/MixerApp.h>

#include <Model/AudioEngine.h>

#include <algorithm>

void MixerApp::play()
{
    if (m_engine) {
        m_engine->play();
    }
}

void MixerApp::pause()
{
    if (m_engine) {
        m_engine->pause();
    }
}

void MixerApp::stop()
{
    if (m_engine) {
        m_engine->stop();
    }
}

void MixerApp::seekToProgress(float progress)
{
    if (!m_engine || m_engine->durationMs() <= 0) {
        return;
    }

    const float clamped = std::clamp(progress, 0.0f, 1.0f);
    m_engine->seek(static_cast<int>(clamped * m_engine->durationMs()));
}

float MixerApp::setMasterVolume(float volume)
{
    const float clamped = std::clamp(volume, 0.0f, 1.0f);
    if (m_engine) {
        m_engine->setMasterVolume(clamped);
    }
    return clamped;
}
