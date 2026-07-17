#pragma once

#include <QString>
#include <QVector>

struct WavDecodeResult
{
    QVector<float> monoSamples;
    int sampleRate = 0;
    int channels = 0;
    int bitsPerSample = 0;
};

// Decodes PCM WAV (8/16/24/32-bit integer or 32-bit float) to mono float [-1,1].
class WavDecoder
{
public:
    static bool decodeFile(const QString &path, WavDecodeResult *out, QString *errorMessage = nullptr);
};
