#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <Model/AudioEngine.h>
#include <ViewModel/MixerViewModel.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    AudioEngine audioEngine;
    MixerViewModel mixerViewModel(&audioEngine);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("mixerViewModel", &mixerViewModel);
    engine.loadFromModule("MixingStudio", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
