#pragma once

#include <Common/ICommandBase.h>
#include <Common/MixerTypes.h>

#include <QString>
#include <QStringList>

class AudioEngine;

class ImportMockTrackCommand final : public ICommand
{
public:
    ImportMockTrackCommand(AudioEngine *engine, int existingTrackCount);
    void execute() override;
    QString trackName() const;

private:
    AudioEngine *m_engine = nullptr;
    int m_existingTrackCount = 0;
    QString m_trackName;
};

class ImportAssetCommand final : public ICommand
{
public:
    ImportAssetCommand(AudioEngine *engine, QString name);
    void execute() override;
    bool ok() const;
    QString status() const;
    QString assetName() const;

private:
    AudioEngine *m_engine = nullptr;
    QString m_name;
    bool m_ok = false;
    QString m_status;
};

class SaveProjectCommand final : public ICommand
{
public:
    SaveProjectCommand(AudioEngine *engine, QString path);
    void execute() override;
    bool ok() const;
    QString status() const;
    QString path() const;

private:
    AudioEngine *m_engine = nullptr;
    QString m_path;
    bool m_ok = false;
    QString m_status;
};

class LoadProjectCommand final : public ICommand
{
public:
    LoadProjectCommand(AudioEngine *engine, QString path);
    void execute() override;
    bool ok() const;
    QString status() const;

private:
    AudioEngine *m_engine = nullptr;
    QString m_path;
    bool m_ok = false;
    QString m_status;
};

class ExportMixCommand final : public ICommand
{
public:
    ExportMixCommand(AudioEngine *engine, QString path, int durationMs = -1);
    void execute() override;
    bool ok() const;
    QString status() const;
    QString path() const;

private:
    AudioEngine *m_engine = nullptr;
    QString m_path;
    int m_durationMs = -1;
    bool m_ok = false;
    QString m_status;
};

class ImportLocalFileCommand final : public ICommand
{
public:
    ImportLocalFileCommand(AudioEngine *engine, QString path);
    void execute() override;
    bool ok() const;
    QString status() const;
    QString displayName() const;

private:
    AudioEngine *m_engine = nullptr;
    QString m_path;
    bool m_ok = false;
    QString m_status;
    QString m_displayName;
};
