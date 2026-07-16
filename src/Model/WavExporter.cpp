#include <Model/WavExporter.h>

#include <QFile>
#include <QIODevice>
#include <QtEndian>

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace {

qint16 floatToPcm16(float sample)
{
    const float clamped = std::clamp(sample, -1.0f, 1.0f);
    const float scaled = clamped < 0.0f ? clamped * 32768.0f : clamped * 32767.0f;
    return static_cast<qint16>(std::lround(scaled));
}

void appendLe32(QByteArray &out, quint32 value)
{
    char bytes[4];
    qToLittleEndian(value, bytes);
    out.append(bytes, 4);
}

void appendLe16(QByteArray &out, quint16 value)
{
    char bytes[2];
    qToLittleEndian(value, bytes);
    out.append(bytes, 2);
}

} // namespace

bool WavExporter::writeStereoPcm16(const QString &path,
                                   const QVector<float> &interleavedStereo,
                                   int sampleRate,
                                   QString *errorMessage)
{
    if (path.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Export path is empty.");
        }
        return false;
    }

    if (interleavedStereo.isEmpty() || (interleavedStereo.size() % 2) != 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Stereo PCM buffer must contain an even number of samples.");
        }
        return false;
    }

    if (sampleRate <= 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid sample rate.");
        }
        return false;
    }

    const quint16 channels = 2;
    const quint16 bitsPerSample = 16;
    const quint32 byteRate = static_cast<quint32>(sampleRate) * channels * (bitsPerSample / 8);
    const quint16 blockAlign = static_cast<quint16>(channels * (bitsPerSample / 8));
    const quint32 dataBytes = static_cast<quint32>(interleavedStereo.size() * static_cast<int>(sizeof(qint16)));

    QByteArray header;
    header.reserve(44);
    header.append("RIFF", 4);
    appendLe32(header, 36 + dataBytes);
    header.append("WAVE", 4);
    header.append("fmt ", 4);
    appendLe32(header, 16);
    appendLe16(header, 1); // PCM
    appendLe16(header, channels);
    appendLe32(header, static_cast<quint32>(sampleRate));
    appendLe32(header, byteRate);
    appendLe16(header, blockAlign);
    appendLe16(header, bitsPerSample);
    header.append("data", 4);
    appendLe32(header, dataBytes);

    QByteArray pcm;
    pcm.resize(static_cast<int>(dataBytes));
    auto *dst = reinterpret_cast<qint16 *>(pcm.data());
    for (int i = 0; i < interleavedStereo.size(); ++i) {
        const qint16 sample = floatToPcm16(interleavedStereo.at(i));
        qToLittleEndian(sample, reinterpret_cast<uchar *>(dst + i));
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Cannot write WAV file: %1").arg(path);
        }
        return false;
    }

    if (file.write(header) != header.size() || file.write(pcm) != pcm.size()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed while writing WAV payload.");
        }
        return false;
    }

    return true;
}
