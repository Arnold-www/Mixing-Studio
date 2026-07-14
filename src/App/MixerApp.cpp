#include <App/MixerApp.h>

#include <Model/AudioEngine.h>

MixerApp::MixerApp(AudioEngine *engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
{
    Q_ASSERT(m_engine != nullptr);

    connect(m_engine, &AudioEngine::playbackStateChanged, this, &MixerApp::playbackStateChanged);
    connect(m_engine, &AudioEngine::positionChanged, this, &MixerApp::positionChanged);
    connect(m_engine, &AudioEngine::durationChanged, this, &MixerApp::durationChanged);
    connect(m_engine, &AudioEngine::masterVolumeChanged, this, &MixerApp::masterVolumeChanged);
    connect(m_engine, &AudioEngine::statusMessageChanged, this, &MixerApp::statusMessageChanged);
    connect(m_engine, &AudioEngine::analysisChanged, this, &MixerApp::analysisChanged);
}

AudioEngine *MixerApp::engine() const
{
    return m_engine;
}

bool MixerApp::isPlaying() const
{
    return m_engine && m_engine->isPlaying();
}

int MixerApp::positionMs() const
{
    return m_engine ? m_engine->positionMs() : 0;
}

int MixerApp::durationMs() const
{
    return m_engine ? m_engine->durationMs() : 0;
}

float MixerApp::masterVolume() const
{
    return m_engine ? m_engine->masterVolume() : 1.0f;
}

QVector<float> MixerApp::waveformPoints() const
{
    return m_engine ? m_engine->waveformPoints() : QVector<float>{};
}

QVector<float> MixerApp::spectrumLevels() const
{
    return m_engine ? m_engine->spectrumLevels() : QVector<float>{};
}

float MixerApp::vuLevel() const
{
    return m_engine ? m_engine->vuLevel() : 0.0f;
}

void MixerApp::refreshAnalysis()
{
    if (m_engine) {
        m_engine->refreshAnalysis();
    }
}
