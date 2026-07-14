#pragma once

#include <Command/ICommand.h>

#include <App/MixerApp.h>

class PlayCommand final : public ICommand
{
public:
    explicit PlayCommand(MixerApp *app);
    void execute() override;

private:
    MixerApp *m_app = nullptr;
};

class PauseCommand final : public ICommand
{
public:
    explicit PauseCommand(MixerApp *app);
    void execute() override;

private:
    MixerApp *m_app = nullptr;
};

class StopCommand final : public ICommand
{
public:
    explicit StopCommand(MixerApp *app);
    void execute() override;

private:
    MixerApp *m_app = nullptr;
};

class SeekProgressCommand final : public ICommand
{
public:
    SeekProgressCommand(MixerApp *app, float progress);
    void execute() override;

private:
    MixerApp *m_app = nullptr;
    float m_progress = 0.0f;
};

class SetMasterVolumeCommand final : public ICommand
{
public:
    SetMasterVolumeCommand(MixerApp *app, float volume);
    void execute() override;
    float clampedVolume() const;

private:
    MixerApp *m_app = nullptr;
    float m_volume = 1.0f;
    float m_clamped = 1.0f;
};
