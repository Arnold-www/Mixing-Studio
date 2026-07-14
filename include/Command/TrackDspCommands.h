#pragma once

#include <Command/ICommand.h>

#include <App/MixerApp.h>

#include <QVector>

class ApplyTrackDspCommand final : public ICommand
{
public:
    ApplyTrackDspCommand(MixerApp *app, int index, TrackDspParams params);
    void execute() override;

private:
    MixerApp *m_app = nullptr;
    int m_index = -1;
    TrackDspParams m_params;
};

class ApplyAllTrackDspCommand final : public ICommand
{
public:
    ApplyAllTrackDspCommand(MixerApp *app, QVector<TrackDspParams> params);
    void execute() override;

private:
    MixerApp *m_app = nullptr;
    QVector<TrackDspParams> m_params;
};
