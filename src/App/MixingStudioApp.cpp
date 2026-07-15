#include <App/MixingStudioApp.h>

#include <Model/AudioEngine.h>
#include <ViewModel/IMixerViewModel.h>
#include <ViewModel/RealMixerViewModel.h>

#include <QQmlContext>

MixingStudioApp::MixingStudioApp() = default;

MixingStudioApp::~MixingStudioApp()
{
    // Owned by Qt parent chain / stack order in run(); nothing extra when run() owns locals.
}

void MixingStudioApp::bindViewModelToQml(QQmlApplicationEngine &engine)
{
    // Interface pointer only — QML must not depend on RealMixerViewModel.
    engine.rootContext()->setContextProperty(QStringLiteral("mixerViewModel"), m_viewModel);
}

int MixingStudioApp::run(int argc, char *argv[])
{
    QGuiApplication guiApp(argc, argv);
    m_guiApp = &guiApp;

    AudioEngine audioEngine;
    m_engine = &audioEngine;

    RealMixerViewModel realViewModel(&audioEngine);
    m_realViewModel = &realViewModel;
    m_viewModel = &realViewModel;

    QQmlApplicationEngine engine;
    bindViewModelToQml(engine);
    engine.loadFromModule("MixingStudio", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return guiApp.exec();
}
