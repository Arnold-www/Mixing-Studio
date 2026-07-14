#include <Model/AudioEngine.h>

#include <Model/AssetLibrary.h>
#include <Model/ProjectStore.h>

#include <QFileInfo>

bool AudioEngine::saveProject(const QString &path)
{
    QString error;
    if (!ProjectStore::saveToFile(*this, path, &error)) {
        emit statusMessageChanged(error);
        return false;
    }
    emit statusMessageChanged(QStringLiteral("Project saved: %1").arg(path));
    return true;
}

bool AudioEngine::loadProject(const QString &path)
{
    QString error;
    if (!ProjectStore::loadFromFile(*this, path, &error)) {
        emit statusMessageChanged(error);
        return false;
    }
    emit statusMessageChanged(QStringLiteral("Project loaded: %1").arg(path));
    return true;
}

bool AudioEngine::openAssetLibrary(const QString &databasePath)
{
    QString error;
    if (!m_assetLibrary->open(databasePath, &error)) {
        emit statusMessageChanged(error);
        return false;
    }
    emit statusMessageChanged(QStringLiteral("Asset library opened."));
    return true;
}

bool AudioEngine::registerAsset(const QString &path, const QString &displayName)
{
    QString error;
    const QString name = displayName.isEmpty() ? QFileInfo(path).fileName() : displayName;
    if (!m_assetLibrary->upsertAsset(path, name, &error)) {
        emit statusMessageChanged(error);
        return false;
    }
    return true;
}

QStringList AudioEngine::searchAssets(const QString &query) const
{
    return m_assetLibrary->search(query);
}

QStringList AudioEngine::recentAssets(int limit) const
{
    return m_assetLibrary->recent(limit);
}
