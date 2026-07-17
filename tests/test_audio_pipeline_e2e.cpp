#include <Model/AudioEngine.h>
#include <Model/WavDecoder.h>
#include <Model/WavExporter.h>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QVector>

#include <cmath>
#include <cstdio>

namespace {

bool check(bool condition, const char *message, int &failures)
{
    if (!condition) {
        std::fprintf(stderr, "%s\n", message);
        ++failures;
    }
    return condition;
}

QVector<float> makeTone(int frames, int sampleRate, float frequency, float amplitude)
{
    QVector<float> stereo(frames * 2);
    for (int i = 0; i < frames; ++i) {
        const float sample = amplitude
            * std::sin(2.0f * 3.14159265358979323846f * frequency * static_cast<float>(i)
                       / static_cast<float>(sampleRate));
        stereo[i * 2] = sample;
        stereo[i * 2 + 1] = sample;
    }
    return stereo;
}

float absoluteEnergy(const QVector<float> &samples)
{
    double energy = 0.0;
    for (float sample : samples) {
        energy += std::fabs(sample);
    }
    return static_cast<float>(energy);
}

} // namespace

int main(int argc, char *argv[])
{
    qputenv("MIXING_STUDIO_FORCE_TIMER_CLOCK", "1");
    QCoreApplication app(argc, argv);
    Q_UNUSED(app);

    int failures = 0;
    QTemporaryDir temp;
    if (!check(temp.isValid(), "temporary directory should be available", failures)) {
        return 1;
    }

    constexpr int sampleRate = 44100;
    constexpr int sourceMs = 400;
    constexpr int frames = sampleRate * sourceMs / 1000;
    QString error;
    const QString sourcePath = QDir(temp.path()).filePath(QStringLiteral("source.wav"));
    check(WavExporter::writeStereoPcm16(sourcePath,
                                       makeTone(frames, sampleRate, 330.0f, 0.4f),
                                       sampleRate,
                                       &error),
          "source WAV generation should succeed",
          failures);

    AudioEngine source;
    check(source.importAudioFile(sourcePath), "engine should import generated WAV", failures);
    check(source.trackCount() == 1, "import should create one track", failures);
    check(std::abs(source.durationMs() - sourceMs) <= 1, "decoded duration should match fixture", failures);

    source.setMasterVolume(0.7f);
    source.setTrackVolume(0, 0.65f);
    source.setTrackPan(0, -0.25f);
    source.setTrackPlaybackRate(0, 0.8f);
    source.setTrackLoopEnabled(0, true);
    source.setTrackLoopRange(0, 0.2f, 0.8f);
    source.setTrackEqBands(0, {0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f});
    source.setTrackCompressor(0, 0.5f, 3.0f);
    source.setTrackVolumeAutomation(0, {{0, 1.0f}, {250, 0.5f}, {500, 0.8f}});
    source.setLoopRange(100, 450);
    source.seek(175);

    const QString projectPath = QDir(temp.path()).filePath(QStringLiteral("roundtrip.json"));
    check(source.saveProject(projectPath), "project save should succeed", failures);
    check(QFileInfo(projectPath).size() > 0, "saved project should not be empty", failures);

    AudioEngine restored;
    check(restored.loadProject(projectPath), "project load should succeed", failures);
    check(restored.trackCount() == 1, "roundtrip should restore track count", failures);
    check(std::abs(restored.masterVolume() - 0.7f) < 0.001f, "roundtrip should restore master volume", failures);
    check(restored.trackData().at(0).loopEnabled, "roundtrip should restore clip loop", failures);
    check(std::abs(restored.trackData().at(0).playbackRate - 0.8f) < 0.001f,
          "roundtrip should restore playback rate",
          failures);
    check(restored.trackData().at(0).volumeAutomation.size() == 3,
          "roundtrip should restore automation",
          failures);
    check(restored.loopStartMs() == 100 && restored.loopEndMs() == 450,
          "roundtrip should restore song loop",
          failures);
    check(restored.positionMs() == 175, "roundtrip should restore playhead", failures);

    const QString exportPath = QDir(temp.path()).filePath(QStringLiteral("export.wav"));
    constexpr int exportMs = 700;
    check(restored.exportMixToWav(exportPath, exportMs), "restored mix export should succeed", failures);

    WavDecodeResult exported;
    check(WavDecoder::decodeFile(exportPath, &exported, &error),
          "exported WAV should decode again",
          failures);
    check(exported.sampleRate == sampleRate, "export sample rate should be 44.1 kHz", failures);
    check(exported.monoSamples.size() == sampleRate * exportMs / 1000,
          "export frame count should match requested duration",
          failures);
    check(absoluteEnergy(exported.monoSamples) > 100.0f,
          "clip-looped export should contain audible samples past source end",
          failures);

    restored.setMasterVolume(0.0f);
    const QString silentPath = QDir(temp.path()).filePath(QStringLiteral("silent.wav"));
    check(restored.exportMixToWav(silentPath, 100), "muted-master export should succeed", failures);
    WavDecodeResult silent;
    check(WavDecoder::decodeFile(silentPath, &silent, &error), "silent export should decode", failures);
    check(absoluteEnergy(silent.monoSamples) == 0.0f, "zero master should export digital silence", failures);

    // Repository-level fixture: validates the shipped JSON's relative paths
    // and every referenced sample WAV as one deliverable.
    AudioEngine sampleSession;
    const QString sampleProject = QDir(QStringLiteral(MIXING_STUDIO_SOURCE_DIR))
                                      .filePath(QStringLiteral("samples/demo_session.json"));
    check(sampleSession.loadProject(sampleProject), "shipped sample project should load", failures);
    check(sampleSession.trackCount() == 4, "shipped sample project should restore four tracks", failures);
    for (const AudioTrack &track : sampleSession.trackData()) {
        check(!track.pcmMono.isEmpty(), "every shipped sample track should decode real PCM", failures);
    }
    const QString sampleExportPath = QDir(temp.path()).filePath(QStringLiteral("sample-session.wav"));
    check(sampleSession.exportMixToWav(sampleExportPath, 1000),
          "shipped sample project should export",
          failures);
    WavDecodeResult sampleExport;
    check(WavDecoder::decodeFile(sampleExportPath, &sampleExport, &error),
          "shipped sample export should decode",
          failures);
    check(absoluteEnergy(sampleExport.monoSamples) > 100.0f,
          "shipped sample export should contain audio",
          failures);

    if (failures == 0) {
        std::printf("test_audio_pipeline_e2e: OK\n");
        return 0;
    }
    std::fprintf(stderr, "test_audio_pipeline_e2e: %d failure(s)\n", failures);
    return 1;
}
