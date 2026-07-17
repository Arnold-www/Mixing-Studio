#pragma once

#include <Common/ICommandBase.h>
#include <Common/MixerTypes.h>

#include <QVector>

class AudioEngine;

class ApplyTrackDspCommand final : public ICommand
{
public:
    ApplyTrackDspCommand(AudioEngine *engine, int index, TrackDspParams params);
    void execute() override;

private:
    AudioEngine *m_engine = nullptr;
    int m_index = -1;
    TrackDspParams m_params;
};

class ApplyAllTrackDspCommand final : public ICommand
{
public:
    ApplyAllTrackDspCommand(AudioEngine *engine, QVector<TrackDspParams> params);
    void execute() override;

private:
    AudioEngine *m_engine = nullptr;
    QVector<TrackDspParams> m_params;
};
