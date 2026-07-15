#pragma once

#include <Common/ICommandBase.h>

class AudioEngine;

class PlayCommand final : public ICommand
{
public:
    explicit PlayCommand(AudioEngine *engine);
    void execute() override;

private:
    AudioEngine *m_engine = nullptr;
};

class PauseCommand final : public ICommand
{
public:
    explicit PauseCommand(AudioEngine *engine);
    void execute() override;

private:
    AudioEngine *m_engine = nullptr;
};

class StopCommand final : public ICommand
{
public:
    explicit StopCommand(AudioEngine *engine);
    void execute() override;

private:
    AudioEngine *m_engine = nullptr;
};

class SeekProgressCommand final : public ICommand
{
public:
    SeekProgressCommand(AudioEngine *engine, float progress);
    void execute() override;

private:
    AudioEngine *m_engine = nullptr;
    float m_progress = 0.0f;
};

class SetMasterVolumeCommand final : public ICommand
{
public:
    SetMasterVolumeCommand(AudioEngine *engine, float volume);
    void execute() override;
    float clampedVolume() const;

private:
    AudioEngine *m_engine = nullptr;
    float m_volume = 1.0f;
    float m_clamped = 1.0f;
};
