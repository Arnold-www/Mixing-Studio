#include <Command/MixerCommands.h>
#include <Model/AudioEngine.h>

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

    ImportMockTrackCommand importCmd(&engine, 0);
    importCmd.execute();
    if (importCmd.trackName().isEmpty() || engine.trackCount() != 1) {
        std::fprintf(stderr, "ImportMockTrackCommand failed\n");
        ++failures;
    }

    PlayCommand(&engine).execute();
    if (!engine.isPlaying()) {
        std::fprintf(stderr, "PlayCommand failed\n");
        ++failures;
    }

    PauseCommand(&engine).execute();
    if (engine.isPlaying()) {
        std::fprintf(stderr, "PauseCommand failed\n");
        ++failures;
    }

    QTemporaryDir temp;
    if (!temp.isValid()) {
        std::fprintf(stderr, "QTemporaryDir failed\n");
        ++failures;
    } else {
        const QString path = QDir(temp.path()).filePath(QStringLiteral("cmd_project.json"));
        SaveProjectCommand saveCmd(&engine, path);
        saveCmd.execute();
        if (!saveCmd.ok()) {
            std::fprintf(stderr, "SaveProjectCommand failed\n");
            ++failures;
        }

        engine.clearTracks();
        LoadProjectCommand loadCmd(&engine, path);
        loadCmd.execute();
        if (!loadCmd.ok() || engine.trackCount() != 1) {
            std::fprintf(stderr, "LoadProjectCommand failed\n");
            ++failures;
        }
    }

    if (failures == 0) {
        std::printf("test_commands: OK\n");
        return 0;
    }

    std::fprintf(stderr, "test_commands: %d failure(s)\n", failures);
    return 1;
}
