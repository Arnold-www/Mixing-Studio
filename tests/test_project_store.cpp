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
    engine.setTrackEqBands(0, {1, 2, 3, 4, 5, -1, -2, -3, 0, 1});
    QVector<AutomationPoint> automation;
    automation.append({0, 1.0f});
    automation.append({1000, 0.25f});
    automation.append({2000, 0.8f});
    engine.setTrackVolumeAutomation(0, automation);
    engine.setMasterVolume(0.75f);
    engine.setLoopRange(1000, 2500);
    engine.seek(1500);

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
    expectTrue(loaded.loopEndMs() == 2500, "Loop end should restore");
    expectTrue(loaded.positionMs() == 1500, "Position should restore");
    expectTrue(loaded.trackData().at(0).volume > 0.59f && loaded.trackData().at(0).volume < 0.61f,
               "Track volume should restore");
    expectTrue(loaded.trackData().at(1).muted, "Track mute should restore");
    expectTrue(loaded.trackData().at(0).eqBandsOrDefault().size() == 10, "EQ bands should restore size");
    expectTrue(std::abs(loaded.trackData().at(0).eqBandsOrDefault().at(4) - 5.0f) < 0.01f,
               "EQ band 4 should restore");
    expectTrue(loaded.trackData().at(0).volumeAutomation.size() == 3, "Automation points should restore");
    expectTrue(std::abs(loaded.trackAutomationVolumeAtMs(0, 1000) - 0.25f) < 0.01f,
               "Automation lerp at knot should restore");
    expectTrue(std::abs(loaded.trackAutomationVolumeAtMs(0, 500) - 0.625f) < 0.02f,
               "Automation lerp midpoint should interpolate");

    return 0;
}
