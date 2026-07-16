#include <Command/ProjectCommands.h>

#include <Model/AudioEngine.h>

#include <QFileInfo>
#include <QUrl>

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

    QString path = m_engine->resolveAssetPath(m_name);
    if (path.isEmpty()) {
        path = m_name;
    }

    if (m_engine->importAudioFile(path)) {
        m_ok = true;
        m_status = QStringLiteral("Imported asset: %1").arg(m_name);
        return;
    }

    m_engine->importTrack(m_name);
    m_ok = true;
    m_status = QStringLiteral("Imported placeholder asset: %1").arg(m_name);
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

ExportMixCommand::ExportMixCommand(AudioEngine *engine, QString path, int durationMs)
    : m_engine(engine)
    , m_path(std::move(path))
    , m_durationMs(durationMs)
{
}

void ExportMixCommand::execute()
{
    if (!m_engine || m_path.trimmed().isEmpty()) {
        m_ok = false;
        m_status = QStringLiteral("Missing engine or export path.");
        return;
    }

    m_ok = m_engine->exportMixToWav(m_path, m_durationMs);
    m_status = m_ok ? QStringLiteral("Exported mix WAV: %1").arg(m_path)
                    : QStringLiteral("Failed to export mix WAV.");
}

bool ExportMixCommand::ok() const
{
    return m_ok;
}

QString ExportMixCommand::status() const
{
    return m_status;
}

QString ExportMixCommand::path() const
{
    return m_path;
}

ImportLocalFileCommand::ImportLocalFileCommand(AudioEngine *engine, QString path)
    : m_engine(engine)
    , m_path(std::move(path))
{
}

void ImportLocalFileCommand::execute()
{
    if (!m_engine || m_path.trimmed().isEmpty()) {
        m_ok = false;
        m_status = QStringLiteral("Choose a WAV file to import.");
        return;
    }

    QString localPath = m_path;
    if (localPath.startsWith(QStringLiteral("file:"), Qt::CaseInsensitive)) {
        localPath = QUrl(localPath).toLocalFile();
    }

    if (!m_engine->importAudioFile(localPath)) {
        m_ok = false;
        m_status = QStringLiteral("Failed to decode audio: %1").arg(localPath);
        return;
    }

    m_engine->registerAsset(localPath);
    m_displayName = QFileInfo(localPath).fileName();
    m_ok = true;
    m_status = QStringLiteral("Imported local audio: %1").arg(m_displayName);
}

bool ImportLocalFileCommand::ok() const
{
    return m_ok;
}

QString ImportLocalFileCommand::status() const
{
    return m_status;
}

QString ImportLocalFileCommand::displayName() const
{
    return m_displayName;
}
