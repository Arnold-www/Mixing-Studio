#pragma once

#include <Model/WavDecoder.h>

#include <QString>

// Decodes WAV (hand-written) and MP3 (Qt Multimedia QAudioDecoder) to mono float.
class AudioFileDecoder
{
public:
    static bool decodeFile(const QString &path, WavDecodeResult *out, QString *errorMessage = nullptr);
};
