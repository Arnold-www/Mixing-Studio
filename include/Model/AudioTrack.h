#pragma once

#include <QString>

struct AudioTrack
{
    QString sourcePath;
    QString displayName;
    float volume = 0.8f;
    float pan = 0.0f;
    bool muted = false;
    bool solo = false;
    float eqLowDb = 0.0f;
    float eqMidDb = 0.0f;
    float eqHighDb = 0.0f;
    float compThreshold = 0.7f;
    float compRatio = 4.0f;
    bool fxBypass = false;
};
