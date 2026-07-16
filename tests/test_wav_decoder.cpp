#include <Model/AudioFileDecoder.h>
#include <Model/WavDecoder.h>
#include <Model/WavExporter.h>

#include <QCoreApplication>
#include <QDir>
#include <QTemporaryDir>
#include <QVector>

#include <cmath>
#include <cstdio>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Q_UNUSED(app);

    int failures = 0;
    QTemporaryDir temp;
    if (!temp.isValid()) {
        std::fprintf(stderr, "temp dir failed\n");
        return 1;
    }

    QVector<float> stereo;
    const int frames = 4410; // 0.1s
    stereo.resize(frames * 2);
    for (int i = 0; i < frames; ++i) {
        const float s = 0.5f * std::sin(2.0f * 3.14159265f * 440.0f * static_cast<float>(i) / 44100.0f);
        stereo[i * 2] = s;
        stereo[i * 2 + 1] = -s;
    }

    const QString path = QDir(temp.path()).filePath(QStringLiteral("tone.wav"));
    QString error;
    if (!WavExporter::writeStereoPcm16(path, stereo, 44100, &error)) {
        std::fprintf(stderr, "export failed: %s\n", qPrintable(error));
        return 1;
    }

    WavDecodeResult decoded;
    if (!WavDecoder::decodeFile(path, &decoded, &error)) {
        std::fprintf(stderr, "decode failed: %s\n", qPrintable(error));
        ++failures;
    } else {
        if (decoded.sampleRate != 44100) {
            std::fprintf(stderr, "sample rate mismatch\n");
            ++failures;
        }
        if (decoded.monoSamples.size() != frames) {
            std::fprintf(stderr, "frame count mismatch %d\n", decoded.monoSamples.size());
            ++failures;
        }
        float energy = 0.0f;
        for (float s : decoded.monoSamples) {
            energy += std::fabs(s);
        }
        if (energy <= 0.0f) {
            std::fprintf(stderr, "decoded energy is zero\n");
            ++failures;
        }
    }

    if (!WavDecoder::decodeFile(QStringLiteral("missing-file.wav"), &decoded, &error)) {
        // expected failure
    } else {
        std::fprintf(stderr, "missing file should fail\n");
        ++failures;
    }

    WavDecodeResult viaFacade;
    if (!AudioFileDecoder::decodeFile(path, &viaFacade, &error)) {
        std::fprintf(stderr, "AudioFileDecoder WAV failed: %s\n", qPrintable(error));
        ++failures;
    } else if (viaFacade.monoSamples.size() != frames) {
        std::fprintf(stderr, "AudioFileDecoder frame mismatch\n");
        ++failures;
    }

    WavDecodeResult unsupported;
    if (AudioFileDecoder::decodeFile(QStringLiteral("x.flac"), &unsupported, &error)) {
        std::fprintf(stderr, "flac should be unsupported\n");
        ++failures;
    }

    if (failures == 0) {
        std::printf("test_wav_decoder: OK\n");
        return 0;
    }
    std::fprintf(stderr, "test_wav_decoder: %d failure(s)\n", failures);
    return 1;
}
