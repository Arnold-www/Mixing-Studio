#include <Model/WavDecoder.h>

#include <QFile>
#include <QIODevice>
#include <QtEndian>

#include <cmath>
#include <cstring>

namespace {

quint16 readU16(const QByteArray &bytes, int offset)
{
    return qFromLittleEndian<quint16>(reinterpret_cast<const uchar *>(bytes.constData() + offset));
}

quint32 readU32(const QByteArray &bytes, int offset)
{
    return qFromLittleEndian<quint32>(reinterpret_cast<const uchar *>(bytes.constData() + offset));
}

qint16 readI16(const QByteArray &bytes, int offset)
{
    return qFromLittleEndian<qint16>(reinterpret_cast<const uchar *>(bytes.constData() + offset));
}

qint32 readI24(const QByteArray &bytes, int offset)
{
    const quint8 b0 = static_cast<quint8>(bytes.at(offset));
    const quint8 b1 = static_cast<quint8>(bytes.at(offset + 1));
    const quint8 b2 = static_cast<quint8>(bytes.at(offset + 2));
    qint32 value = (static_cast<qint32>(b2) << 16) | (static_cast<qint32>(b1) << 8) | b0;
    if (value & 0x800000) {
        value |= ~0xFFFFFF;
    }
    return value;
}

float sampleFromPcm(const QByteArray &data, int offset, int bitsPerSample, bool isFloat)
{
    if (isFloat && bitsPerSample == 32) {
        float value = 0.0f;
        std::memcpy(&value, data.constData() + offset, sizeof(float));
        return value;
    }

    switch (bitsPerSample) {
    case 8:
        return (static_cast<quint8>(data.at(offset)) - 128) / 128.0f;
    case 16:
        return readI16(data, offset) / 32768.0f;
    case 24:
        return readI24(data, offset) / 8388608.0f;
    case 32:
        return qFromLittleEndian<qint32>(reinterpret_cast<const uchar *>(data.constData() + offset))
            / 2147483648.0f;
    default:
        return 0.0f;
    }
}

} // namespace

bool WavDecoder::decodeFile(const QString &path, WavDecodeResult *out, QString *errorMessage)
{
    if (!out) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Decode output is null.");
        }
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Cannot open WAV: %1").arg(path);
        }
        return false;
    }

    const QByteArray bytes = file.readAll();
    if (bytes.size() < 44 || bytes.mid(0, 4) != "RIFF" || bytes.mid(8, 4) != "WAVE") {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Not a RIFF/WAVE file: %1").arg(path);
        }
        return false;
    }

    int offset = 12;
    int audioFormat = 0;
    int channels = 0;
    int sampleRate = 0;
    int bitsPerSample = 0;
    int dataOffset = -1;
    int dataSize = 0;

    while (offset + 8 <= bytes.size()) {
        const QByteArray chunkId = bytes.mid(offset, 4);
        const quint32 chunkSize = readU32(bytes, offset + 4);
        const int chunkData = offset + 8;
        if (chunkData + static_cast<int>(chunkSize) > bytes.size()) {
            break;
        }

        if (chunkId == "fmt ") {
            if (chunkSize < 16) {
                if (errorMessage) {
                    *errorMessage = QStringLiteral("Invalid fmt chunk.");
                }
                return false;
            }
            audioFormat = readU16(bytes, chunkData);
            channels = readU16(bytes, chunkData + 2);
            sampleRate = static_cast<int>(readU32(bytes, chunkData + 4));
            bitsPerSample = readU16(bytes, chunkData + 14);
        } else if (chunkId == "data") {
            dataOffset = chunkData;
            dataSize = static_cast<int>(chunkSize);
        }

        offset = chunkData + static_cast<int>(chunkSize);
        if (chunkSize & 1) {
            ++offset;
        }
    }

    const bool isFloat = (audioFormat == 3);
    const bool isPcm = (audioFormat == 1);
    const bool supportedPcmDepth = bitsPerSample == 8 || bitsPerSample == 16
        || bitsPerSample == 24 || bitsPerSample == 32;
    const bool supportedFloatDepth = bitsPerSample == 32;
    if ((!isPcm && !isFloat) || channels <= 0 || sampleRate <= 0 || dataOffset < 0
        || (isPcm && !supportedPcmDepth) || (isFloat && !supportedFloatDepth)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Unsupported or incomplete WAV format.");
        }
        return false;
    }

    const int bytesPerSample = bitsPerSample / 8;
    if (bytesPerSample <= 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid bits per sample.");
        }
        return false;
    }

    const int frameBytes = bytesPerSample * channels;
    if (frameBytes <= 0 || dataSize < frameBytes) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("WAV data chunk is empty.");
        }
        return false;
    }

    const int frames = dataSize / frameBytes;
    out->monoSamples.resize(frames);
    out->sampleRate = sampleRate;
    out->channels = channels;
    out->bitsPerSample = bitsPerSample;

    for (int i = 0; i < frames; ++i) {
        float sum = 0.0f;
        const int frameOffset = dataOffset + i * frameBytes;
        for (int ch = 0; ch < channels; ++ch) {
            sum += sampleFromPcm(bytes, frameOffset + ch * bytesPerSample, bitsPerSample, isFloat);
        }
        out->monoSamples[i] = sum / static_cast<float>(channels);
    }

    return true;
}
