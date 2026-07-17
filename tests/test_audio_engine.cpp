#include <Model/AudioEngine.h>

#include <QCoreApplication>
#include <QEventLoop>
#include <QTimer>
#include <QVector>

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
    qputenv("MIXING_STUDIO_FORCE_TIMER_CLOCK", "1");
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
    expectEqual(engine.durationMs(), 3000, "First import should set placeholder duration");
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
    expectEqual(engine.positionMs(), 3000, "Seek should clamp to duration");

    engine.seek(1500);
    expectEqual(engine.positionMs(), 1500, "Seek should store in-range position");

    engine.setLoopRange(500, 2000);
    expectEqual(engine.loopStartMs(), 500, "Loop start should be stored");
    expectEqual(engine.loopEndMs(), 2000, "Loop end should be stored");

    engine.play();
    engine.seek(1950);
    QTimer::singleShot(120, &loop, &QEventLoop::quit);
    loop.exec();
    expectTrue(engine.positionMs() < 2000, "Playback should wrap inside loop range");
    expectTrue(engine.positionMs() >= 500, "Loop wrap should land at or after loop start");

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

    engine.importTrack("Kick.wav");
    engine.importTrack("Snare.wav");
    engine.importTrack("Hat.wav");
    expectEqual(engine.trackCount(), 3, "Three tracks should be present before remove");
    expectTrue(engine.removeTrack(1), "Removing middle track should succeed");
    expectEqual(engine.trackCount(), 2, "Track count should drop after remove");
    expectTrue(!engine.removeTrack(5), "Invalid remove index should fail");
    engine.clearTracks();

    // Stage 3: track DSP params + offline mix / mute / solo / limiter path
    engine.importTrack("Kick.wav");
    engine.importTrack("Snare.wav");
    engine.setTrackVolume(0, 1.0f);
    engine.setTrackVolume(1, 1.0f);
    engine.setTrackPan(0, -1.0f);
    engine.setTrackPan(1, 1.0f);
    engine.setTrackFxBypass(0, true);
    engine.setTrackFxBypass(1, true);
    engine.setMasterVolume(1.0f);

    QVector<float> inputs{0.5f, 0.5f};
    StereoSample mixed = engine.renderMixFrame(inputs);
    expectNear(mixed.left, 0.5f, "Left-panned track should dominate left bus");
    expectNear(mixed.right, 0.5f, "Right-panned track should dominate right bus");

    engine.setTrackMuted(0, true);
    mixed = engine.renderMixFrame(inputs);
    expectNear(mixed.left, 0.0f, "Muted left track should silence left bus");
    expectNear(mixed.right, 0.5f, "Right track should remain after mute");

    engine.setTrackMuted(0, false);
    engine.setTrackSolo(1, true);
    expectTrue(engine.trackAudible(1), "Soloed track should stay audible");
    expectTrue(!engine.trackAudible(0), "Non-solo track should be inaudible while solo active");
    mixed = engine.renderMixFrame(inputs);
    expectNear(mixed.left, 0.0f, "Solo should silence non-solo left track");
    expectNear(mixed.right, 0.5f, "Soloed right track should remain");

    engine.setTrackSolo(1, false);
    engine.setTrackFxBypass(0, false);
    engine.setTrackEq(0, 6.0f, 0.0f, 0.0f);
    engine.setTrackPan(0, 0.0f);
    engine.setTrackPan(1, 0.0f);
    engine.setTrackVolume(1, 0.0f);
    mixed = engine.renderMixFrame(QVector<float>{0.2f, 0.0f});
    expectTrue(mixed.left > 0.2f, "Positive EQ without bypass should raise level");

    engine.setTrackEq(0, 0.0f, 0.0f, 0.0f);
    engine.setTrackCompressor(0, 0.3f, 4.0f);
    mixed = engine.renderMixFrame(QVector<float>{1.0f, 0.0f});
    expectTrue(mixed.left < 1.0f, "Compressor should reduce hot samples");

    engine.setMasterVolume(1.0f);
    engine.setTrackFxBypass(0, true);
    engine.setTrackVolume(0, 1.0f);
    engine.setTrackVolume(1, 1.0f);
    mixed = engine.renderMixFrame(QVector<float>{1.0f, 1.0f});
    expectNear(mixed.left, 1.0f, "Master limiter should clamp summed peaks");
    expectNear(mixed.right, 1.0f, "Master limiter should clamp right peaks");

    // Stage 4 analysis refresh on placeholder buffer
    engine.refreshAnalysis();
    expectTrue(engine.waveformPoints().size() == 256, "Waveform should expose 256 bins");
    expectTrue(engine.spectrumLevels().size() == 32, "Spectrum should expose 32 bands");
    expectTrue(engine.peakLevel() >= 0.0f, "Peak level should be non-negative");

    return 0;
}
