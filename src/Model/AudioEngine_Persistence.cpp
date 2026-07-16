#include <Model/AudioEngine.h>

#include <Model/AssetLibrary.h>
#include <Model/ProjectStore.h>
#include <Model/WavExporter.h>

#include <QFileInfo>
#include <QVector>

#include <cmath>

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

bool AudioEngine::exportMixToWav(const QString &path, int durationMs)
{
    if (m_tracks.isEmpty()) {
        emit statusMessageChanged(QStringLiteral("Import at least one track before export."));
        return false;
    }

    int exportMs = durationMs;
    if (exportMs < 0) {
        exportMs = m_durationMs > 0 ? m_durationMs : kPlaceholderDurationMs;
    }
    if (exportMs <= 0) {
        emit statusMessageChanged(QStringLiteral("Export duration must be positive."));
        return false;
    }

    const qint64 frameCount = (static_cast<qint64>(exportMs) * kExportSampleRate) / 1000;
    if (frameCount <= 0) {
        emit statusMessageChanged(QStringLiteral("Export produced zero frames."));
        return false;
    }

    QVector<float> interleaved;
    interleaved.resize(static_cast<int>(frameCount * 2));

    for (qint64 frame = 0; frame < frameCount; ++frame) {
        const double seconds = static_cast<double>(frame) / static_cast<double>(kExportSampleRate);
        const StereoSample mixed = renderMixAtSeconds(seconds, false);
        const int base = static_cast<int>(frame * 2);
        interleaved[base] = mixed.left;
        interleaved[base + 1] = mixed.right;
    }

    QString error;
    if (!WavExporter::writeStereoPcm16(path, interleaved, kExportSampleRate, &error)) {
        emit statusMessageChanged(error);
        return false;
    }

    emit statusMessageChanged(QStringLiteral("Exported mix WAV: %1").arg(path));
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

QString AudioEngine::resolveAssetPath(const QString &displayName) const
{
    return m_assetLibrary->pathForName(displayName);
}
