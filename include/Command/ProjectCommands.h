#pragma once

#include <Command/ICommand.h>

#include <App/MixerApp.h>

#include <QString>
#include <QStringList>

class ImportMockTrackCommand final : public ICommand
{
public:
    ImportMockTrackCommand(MixerApp *app, int existingTrackCount);
    void execute() override;
    QString trackName() const;

private:
    MixerApp *m_app = nullptr;
    int m_existingTrackCount = 0;
    QString m_trackName;
};

class ImportAssetCommand final : public ICommand
{
public:
    ImportAssetCommand(MixerApp *app, QString name);
    void execute() override;
    bool ok() const;
    QString status() const;
    QString assetName() const;

private:
    MixerApp *m_app = nullptr;
    QString m_name;
    bool m_ok = false;
    QString m_status;
};

class SaveMockProjectCommand final : public ICommand
{
public:
    SaveMockProjectCommand(MixerApp *app, int trackCount, QStringList recent);
    void execute() override;
    QStringList recentProjects() const;
    QString status() const;

private:
    MixerApp *m_app = nullptr;
    int m_trackCount = 0;
    QStringList m_recent;
    QString m_status;
};

class RestoreRecentProjectCommand final : public ICommand
{
public:
    RestoreRecentProjectCommand(MixerApp *app, QString name);
    void execute() override;
    QString status() const;

private:
    MixerApp *m_app = nullptr;
    QString m_name;
    QString m_status;
};
