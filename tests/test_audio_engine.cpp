#include <Model/AudioEngine.h>

#include <QCoreApplication>
#include <QEventLoop>
#include <QTimer>

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

void expectEqual(int actual, int expected, const char *message)
{
    if (actual != expected) {
        std::cerr << message << " expected " << expected << " but got " << actual << '\n';
        std::exit(1);
    }
}

void expectNear(float actual, float expected, const char *message)
{
    constexpr float epsilon = 0.0001f;
    if (std::abs(actual - expected) > epsilon) {
        std::cerr << message << " expected " << expected << " but got " << actual << '\n';
        std::exit(1);
    }
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    AudioEngine engine;
    expectEqual(engine.trackCount(), 0, "New engine should start empty");
    expectTrue(!engine.isPlaying(), "New engine should not be playing");
    expectEqual(engine.positionMs(), 0, "New engine position should be zero");
    expectEqual(engine.durationMs(), 0, "New engine duration should be zero");

    engine.play();
    expectTrue(!engine.isPlaying(), "Engine should not play without tracks");

    engine.importTrack("Lead Vocal.wav");
    engine.importTrack("Backing Vocal.wav");
    expectEqual(engine.trackCount(), 2, "Imported tracks should be counted");
    expectEqual(engine.tracks().size(), 2, "Track name list should match count");
    expectEqual(engine.durationMs(), 180000, "First import should set placeholder duration");
    expectTrue(engine.trackData().at(0).sourcePath == QStringLiteral("Lead Vocal.wav"),
               "Track source path should be stored");
    expectTrue(engine.trackData().at(0).displayName == QStringLiteral("Lead Vocal.wav"),
               "Track display name should use file name");

    engine.play();
    expectTrue(engine.isPlaying(), "Engine should play after importing tracks");

    const int startPosition = engine.positionMs();
    QEventLoop loop;
    QTimer::singleShot(150, &loop, &QEventLoop::quit);
    loop.exec();
    expectTrue(engine.positionMs() > startPosition, "Playback timer should advance position");

    engine.pause();
    expectTrue(!engine.isPlaying(), "Pause should stop playback");
    const int pausedPosition = engine.positionMs();

    QTimer::singleShot(80, &loop, &QEventLoop::quit);
    loop.exec();
    expectEqual(engine.positionMs(), pausedPosition, "Paused engine should not advance");

    engine.seek(200000);
    expectEqual(engine.positionMs(), 180000, "Seek should clamp to duration");

    engine.seek(45000);
    expectEqual(engine.positionMs(), 45000, "Seek should store in-range position");

    engine.setLoopRange(30000, 90000);
    expectEqual(engine.loopStartMs(), 30000, "Loop start should be stored");
    expectEqual(engine.loopEndMs(), 90000, "Loop end should be stored");

    engine.play();
    engine.seek(89500);
    QTimer::singleShot(120, &loop, &QEventLoop::quit);
    loop.exec();
    expectTrue(engine.positionMs() < 90000, "Playback should wrap inside loop range");
    expectTrue(engine.positionMs() >= 30000, "Loop wrap should land at or after loop start");

    engine.setMasterVolume(1.5f);
    expectNear(engine.masterVolume(), 1.0f, "Master volume should clamp high values");
    engine.setMasterVolume(-0.2f);
    expectNear(engine.masterVolume(), 0.0f, "Master volume should clamp low values");

    engine.stop();
    expectTrue(!engine.isPlaying(), "Stop should clear playing state");
    expectEqual(engine.positionMs(), 0, "Stop should reset position");

    engine.clearTracks();
    expectEqual(engine.trackCount(), 0, "Clear should remove all tracks");
    expectEqual(engine.durationMs(), 0, "Clear should reset duration");

    return 0;
}
