#include <Model/AudioFileDecoder.h>

#include <QAudioBuffer>
#include <QAudioDecoder>
#include <QAudioFormat>
#include <QEventLoop>
#include <QFileInfo>
#include <QObject>
#include <QTimer>
#include <QUrl>

#include <algorithm>
#include <cmath>

namespace {

float sampleFromBuffer(const QAudioBuffer &buffer, int frame, int channel)
{
    const QAudioFormat format = buffer.format();
    if (format.sampleFormat() == QAudioFormat::Float) {
        const float *data = buffer.constData<float>();
        return data[frame * format.channelCount() + channel];
    }
    if (format.sampleFormat() == QAudioFormat::Int16) {
        const qint16 *data = buffer.constData<qint16>();
        return data[frame * format.channelCount() + channel] / 32768.0f;
    }
    if (format.sampleFormat() == QAudioFormat::Int32) {
        const qint32 *data = buffer.constData<qint32>();
        return data[frame * format.channelCount() + channel] / 2147483648.0f;
    }
    if (format.sampleFormat() == QAudioFormat::UInt8) {
        const quint8 *data = buffer.constData<quint8>();
        return (data[frame * format.channelCount() + channel] - 128) / 128.0f;
    }
    return 0.0f;
}

bool decodeWithQtMultimedia(const QString &path, WavDecodeResult *out, QString *errorMessage)
{
    QAudioDecoder decoder;
    decoder.setSource(QUrl::fromLocalFile(path));

    QEventLoop loop;
    QString errorText;
    bool finishedOk = false;

    QObject::connect(&decoder, &QAudioDecoder::bufferReady, &decoder, [&]() {
        while (decoder.bufferAvailable()) {
            const QAudioBuffer buffer = decoder.read();
            if (!buffer.isValid() || buffer.frameCount() <= 0) {
                continue;
            }

            if (out->sampleRate <= 0) {
                out->sampleRate = buffer.format().sampleRate();
                out->channels = buffer.format().channelCount();
                out->bitsPerSample = buffer.format().bytesPerSample() * 8;
            }

            const int channels = std::max(1, buffer.format().channelCount());
            const int start = out->monoSamples.size();
            out->monoSamples.resize(start + buffer.frameCount());
            for (int i = 0; i < buffer.frameCount(); ++i) {
                float sum = 0.0f;
                for (int ch = 0; ch < channels; ++ch) {
                    sum += sampleFromBuffer(buffer, i, ch);
                }
                out->monoSamples[start + i] = sum / static_cast<float>(channels);
            }
        }
    });

    QObject::connect(&decoder, &QAudioDecoder::finished, &loop, [&]() {
        finishedOk = true;
        loop.quit();
    });
    QObject::connect(&decoder, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error), &loop, [&](QAudioDecoder::Error) {
        errorText = decoder.errorString();
        loop.quit();
    });

    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, [&]() {
        errorText = QStringLiteral("Timed out decoding audio file.");
        loop.quit();
    });
    timeout.start(30000);

    decoder.start();
    loop.exec();

    if (!finishedOk || out->monoSamples.isEmpty()) {
        if (errorMessage) {
            *errorMessage = errorText.isEmpty() ? QStringLiteral("Failed to decode audio file.") : errorText;
        }
        return false;
    }

    if (out->sampleRate <= 0) {
        out->sampleRate = 44100;
    }
    return true;
}

} // namespace

bool AudioFileDecoder::decodeFile(const QString &path, WavDecodeResult *out, QString *errorMessage)
{
    if (!out) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Decode output is null.");
        }
        return false;
    }

    out->monoSamples.clear();
    out->sampleRate = 0;
    out->channels = 0;
    out->bitsPerSample = 0;

    const QString suffix = QFileInfo(path).suffix().toLower();
    if (suffix == QStringLiteral("wav") || suffix == QStringLiteral("wave")) {
        return WavDecoder::decodeFile(path, out, errorMessage);
    }

    if (suffix == QStringLiteral("mp3")) {
        return decodeWithQtMultimedia(path, out, errorMessage);
    }

    if (errorMessage) {
        *errorMessage = QStringLiteral("Unsupported audio format: %1").arg(suffix);
    }
    return false;
}
