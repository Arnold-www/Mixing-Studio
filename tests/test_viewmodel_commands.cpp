#include <Model/AudioEngine.h>
#include <ViewModel/RealMixerViewModel.h>

#include <QCoreApplication>
#include <QDir>
#include <QTemporaryDir>

#include <cstdio>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Q_UNUSED(app);

    int failures = 0;

    AudioEngine engine;
    RealMixerViewModel viewModel(&engine);

    viewModel.importMockTrack();
    if (engine.trackCount() != 1) {
        std::fprintf(stderr, "importMockTrack failed\n");
        ++failures;
    }

    viewModel.play();
    if (!engine.isPlaying()) {
        std::fprintf(stderr, "play slot failed\n");
        ++failures;
    }

    viewModel.pause();
    if (engine.isPlaying()) {
        std::fprintf(stderr, "pause slot failed\n");
        ++failures;
    }

    QTemporaryDir temp;
    if (!temp.isValid()) {
        std::fprintf(stderr, "QTemporaryDir failed\n");
        ++failures;
    } else {
        // saveProject writes under AppData; exercise engine path via ViewModel export Mix + reload via load.
        viewModel.stop();
        const QString path = QDir(temp.path()).filePath(QStringLiteral("vm_project.json"));
        if (!engine.saveProject(path)) {
            std::fprintf(stderr, "engine.saveProject failed\n");
            ++failures;
        } else {
            engine.clearTracks();
            if (!engine.loadProject(path) || engine.trackCount() != 1) {
                std::fprintf(stderr, "engine.loadProject failed\n");
                ++failures;
            }
        }

        viewModel.setMasterVolume(0.5f);
        if (!qFuzzyCompare(engine.masterVolume(), 0.5f)) {
            std::fprintf(stderr, "setMasterVolume slot failed\n");
            ++failures;
        }
    }

    if (failures == 0) {
        std::printf("test_viewmodel_commands: OK\n");
        return 0;
    }

    std::fprintf(stderr, "test_viewmodel_commands: %d failure(s)\n", failures);
    return 1;
}
