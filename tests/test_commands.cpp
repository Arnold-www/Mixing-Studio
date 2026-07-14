#include <App/MixerApp.h>
#include <Command/MixerCommands.h>
#include <Model/AudioEngine.h>

#include <QCoreApplication>
#include <cstdio>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Q_UNUSED(app);

    int failures = 0;

    AudioEngine engine;
    MixerApp mixerApp(&engine);

    ImportMockTrackCommand importCmd(&mixerApp, 0);
    importCmd.execute();
    if (importCmd.trackName().isEmpty() || engine.trackCount() != 1) {
        std::fprintf(stderr, "ImportMockTrackCommand failed\n");
        ++failures;
    }

    PlayCommand(&mixerApp).execute();
    if (!mixerApp.isPlaying()) {
        std::fprintf(stderr, "PlayCommand failed\n");
        ++failures;
    }

    PauseCommand(&mixerApp).execute();
    if (mixerApp.isPlaying()) {
        std::fprintf(stderr, "PauseCommand failed\n");
        ++failures;
    }

    if (failures == 0) {
        std::printf("test_commands: OK\n");
        return 0;
    }

    std::fprintf(stderr, "test_commands: %d failure(s)\n", failures);
    return 1;
}
