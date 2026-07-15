#include <Command/ProjectCommands.h>

#include <Model/AudioEngine.h>

#include <utility>

ImportMockTrackCommand::ImportMockTrackCommand(AudioEngine *engine, int existingTrackCount)
    : m_engine(engine)
    , m_existingTrackCount(existingTrackCount)
{
}

void ImportMockTrackCommand::execute()
{
    m_trackName = QStringLiteral("Track %1").arg(m_existingTrackCount + 1);
    if (m_engine) {
        m_engine->importTrack(m_trackName);
    }
}

QString ImportMockTrackCommand::trackName() const
{
    return m_trackName;
}

ImportAssetCommand::ImportAssetCommand(AudioEngine *engine, QString name)
    : m_engine(engine)
    , m_name(std::move(name))
{
}

void ImportAssetCommand::execute()
{
    if (m_name.trimmed().isEmpty()) {
        m_ok = false;
        m_status = QStringLiteral("Select an asset before importing.");
        return;
    }

    if (!m_engine) {
        m_ok = false;
        m_status = QStringLiteral("Audio engine unavailable.");
        return;
    }

    m_engine->registerAsset(m_name, m_name);
    m_engine->importTrack(m_name);
    m_ok = true;
    m_status = QStringLiteral("Imported asset: %1").arg(m_name);
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

SaveProjectCommand::SaveProjectCommand(AudioEngine *engine, QString path)
    : m_engine(engine)
    , m_path(std::move(path))
{
}

void SaveProjectCommand::execute()
{
    if (!m_engine || m_path.trimmed().isEmpty()) {
        m_ok = false;
        m_status = QStringLiteral("Missing engine or project path.");
        return;
    }

    if (m_engine->trackCount() <= 0) {
        m_ok = false;
        m_status = QStringLiteral("Add at least one track before saving.");
        return;
    }

    m_ok = m_engine->saveProject(m_path);
    m_status = m_ok ? QStringLiteral("Project saved: %1").arg(m_path)
                    : QStringLiteral("Failed to save project.");
}

bool SaveProjectCommand::ok() const
{
    return m_ok;
}

QString SaveProjectCommand::status() const
{
    return m_status;
}

QString SaveProjectCommand::path() const
{
    return m_path;
}

LoadProjectCommand::LoadProjectCommand(AudioEngine *engine, QString path)
    : m_engine(engine)
    , m_path(std::move(path))
{
}

void LoadProjectCommand::execute()
{
    if (!m_engine || m_path.trimmed().isEmpty()) {
        m_ok = false;
        m_status = QStringLiteral("Select a recent project before restore.");
        return;
    }

    m_ok = m_engine->loadProject(m_path);
    m_status = m_ok ? QStringLiteral("Project loaded: %1").arg(m_path)
                    : QStringLiteral("Failed to load project.");
}

bool LoadProjectCommand::ok() const
{
    return m_ok;
}

QString LoadProjectCommand::status() const
{
    return m_status;
}
