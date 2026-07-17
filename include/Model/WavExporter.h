#pragma once

#include <QString>
#include <QVector>

// Writes interleaved stereo float samples [-1,1] as 16-bit PCM WAV.
class WavExporter
{
public:
    static constexpr int kDefaultSampleRate = 44100;

    static bool writeStereoPcm16(const QString &path,
                                 const QVector<float> &interleavedStereo,
                                 int sampleRate = kDefaultSampleRate,
                                 QString *errorMessage = nullptr);
};
