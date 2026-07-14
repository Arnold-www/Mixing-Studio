#include <Command/ProjectCommands.h>

#include <utility>

ImportMockTrackCommand::ImportMockTrackCommand(MixerApp *app, int existingTrackCount)
    : m_app(app)
    , m_existingTrackCount(existingTrackCount)
{
}

void ImportMockTrackCommand::execute()
{
    if (m_app) {
        m_trackName = m_app->importMockTrack(m_existingTrackCount);
    }
}

QString ImportMockTrackCommand::trackName() const
{
    return m_trackName;
}

ImportAssetCommand::ImportAssetCommand(MixerApp *app, QString name)
    : m_app(app)
    , m_name(std::move(name))
{
}

void ImportAssetCommand::execute()
{
    if (!m_app) {
        m_ok = false;
        m_status = QStringLiteral("App unavailable.");
        return;
    }

    m_ok = m_app->importAssetByName(m_name, &m_status);
}

bool ImportAssetCommand::ok() const
{
    return m_ok;
}

QString ImportAssetCommand::status() const
{
    return m_status;
}

QString ImportAssetCommand::assetName() const
{
    return m_name;
}

SaveMockProjectCommand::SaveMockProjectCommand(MixerApp *app, int trackCount, QStringList recent)
    : m_app(app)
    , m_trackCount(trackCount)
    , m_recent(std::move(recent))
{
}

void SaveMockProjectCommand::execute()
{
    if (!m_app) {
        m_status = QStringLiteral("App unavailable.");
        return;
    }

    m_recent = m_app->saveMockSnapshot(m_trackCount, m_recent, &m_status);
}

QStringList SaveMockProjectCommand::recentProjects() const
{
    return m_recent;
}

QString SaveMockProjectCommand::status() const
{
    return m_status;
}

RestoreRecentProjectCommand::RestoreRecentProjectCommand(MixerApp *app, QString name)
    : m_app(app)
    , m_name(std::move(name))
{
}

void RestoreRecentProjectCommand::execute()
{
    if (!m_app) {
        m_status = QStringLiteral("App unavailable.");
        return;
    }

    m_app->restoreRecentProject(m_name, &m_status);
}

QString RestoreRecentProjectCommand::status() const
{
    return m_status;
}
