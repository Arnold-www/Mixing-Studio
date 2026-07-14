#include <Model/ProjectStore.h>

#include <QCoreApplication>
#include <QTemporaryDir>

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {

void expectTrue(bool condition, const char *message)
{
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    AudioEngine engine;
    engine.importTrack("Kick.wav");
    engine.importTrack("Snare.wav");
    engine.setTrackVolume(0, 0.6f);
    engine.setTrackPan(1, 0.5f);
    engine.setTrackMuted(1, true);
    engine.setMasterVolume(0.75f);
    engine.setLoopRange(1000, 5000);
    engine.seek(2500);

    QTemporaryDir temp;
    expectTrue(temp.isValid(), "Temp dir should be valid");
    const QString path = temp.filePath(QStringLiteral("session.json"));

    QString error;
    expectTrue(ProjectStore::saveToFile(engine, path, &error), "Save project should succeed");

    AudioEngine loaded;
    expectTrue(ProjectStore::loadFromFile(loaded, path, &error), "Load project should succeed");
    expectTrue(loaded.trackCount() == 2, "Loaded project should restore two tracks");
    expectTrue(std::abs(loaded.masterVolume() - 0.75f) < 0.0001f, "Master volume should restore");
    expectTrue(loaded.loopStartMs() == 1000, "Loop start should restore");
    expectTrue(loaded.loopEndMs() == 5000, "Loop end should restore");
    expectTrue(loaded.positionMs() == 2500, "Position should restore");
    expectTrue(loaded.trackData().at(0).volume > 0.59f && loaded.trackData().at(0).volume < 0.61f,
               "Track volume should restore");
    expectTrue(loaded.trackData().at(1).muted, "Track mute should restore");

    return 0;
}
