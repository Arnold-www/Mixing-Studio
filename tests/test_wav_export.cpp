#include <Model/AudioEngine.h>
#include <Model/WavExporter.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QTemporaryDir>

#include <cstdio>

namespace {

bool expectTrue(bool condition, const char *message, int *failures)
{
    if (!condition) {
        std::fprintf(stderr, "%s\n", message);
        ++(*failures);
        return false;
    }
    return true;
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Q_UNUSED(app);

    int failures = 0;
    QTemporaryDir temp;
    if (!expectTrue(temp.isValid(), "temp dir", &failures)) {
        return 1;
    }

    // Direct writer smoke test.
    {
        QVector<float> stereo{0.0f, 0.0f, 0.5f, -0.5f, 1.0f, -1.0f};
        const QString path = QDir(temp.path()).filePath(QStringLiteral("direct.wav"));
        QString error;
        expectTrue(WavExporter::writeStereoPcm16(path, stereo, 44100, &error), "direct write", &failures);
        QFile file(path);
        expectTrue(file.open(QIODevice::ReadOnly), "open direct.wav", &failures);
        const QByteArray bytes = file.readAll();
        expectTrue(bytes.size() == 44 + 6 * 2, "direct wav size", &failures);
        expectTrue(bytes.startsWith("RIFF"), "riff header", &failures);
        expectTrue(bytes.mid(8, 4) == "WAVE", "wave marker", &failures);
    }

    AudioEngine engine;
    expectTrue(!engine.exportMixToWav(QDir(temp.path()).filePath(QStringLiteral("empty.wav")), 100),
               "export without tracks should fail",
               &failures);

    engine.importTrack(QStringLiteral("Lead.wav"));
    engine.importTrack(QStringLiteral("Bass.wav"));
    engine.setTrackVolume(0, 0.9f);
    engine.setTrackPan(1, -0.5f);
    engine.setMasterVolume(0.8f);

    const QString mixPath = QDir(temp.path()).filePath(QStringLiteral("mix.wav"));
    expectTrue(engine.exportMixToWav(mixPath, 200), "export 200ms mix", &failures);

    QFile mixFile(mixPath);
    expectTrue(mixFile.open(QIODevice::ReadOnly), "open mix.wav", &failures);
    const QByteArray mixBytes = mixFile.readAll();
    const int expectedDataBytes = (44100 * 200 / 1000) * 2 * 2; // frames * channels * sizeof(int16)
    expectTrue(mixBytes.size() == 44 + expectedDataBytes, "mix wav payload size", &failures);
    expectTrue(mixBytes.startsWith("RIFF"), "mix riff", &failures);

    if (failures == 0) {
        std::printf("test_wav_export: OK\n");
        return 0;
    }

    std::fprintf(stderr, "test_wav_export: %d failure(s)\n", failures);
    return 1;
}
