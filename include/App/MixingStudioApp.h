#pragma once

#include <QGuiApplication>
#include <QQmlApplicationEngine>

class AudioEngine;
class IMixerViewModel;
class RealMixerViewModel;

// App assembly layer: create Model/ViewModel and inject interface pointer into QML.
class MixingStudioApp
{
public:
    MixingStudioApp();
    ~MixingStudioApp();

    MixingStudioApp(const MixingStudioApp &) = delete;
    MixingStudioApp &operator=(const MixingStudioApp &) = delete;

    int run(int argc, char *argv[]);

private:
    void bindViewModelToQml(QQmlApplicationEngine &engine);

    QGuiApplication *m_guiApp = nullptr;
    AudioEngine *m_engine = nullptr;
    RealMixerViewModel *m_realViewModel = nullptr;
    IMixerViewModel *m_viewModel = nullptr;
};
