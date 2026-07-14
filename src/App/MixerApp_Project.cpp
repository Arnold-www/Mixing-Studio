#include <App/MixerApp.h>

#include <Model/AudioEngine.h>

#include <algorithm>

QStringList MixerApp::defaultAssetCatalog() const
{
    return {
        QStringLiteral("Lead Vocal.wav"),
        QStringLiteral("Backing Vocals.wav"),
        QStringLiteral("Acoustic Guitar.wav"),
        QStringLiteral("Electric Guitar DI.wav"),
        QStringLiteral("Bass DI.wav"),
        QStringLiteral("Kick Close.wav"),
        QStringLiteral("Snare Top.wav"),
        QStringLiteral("Drum Overheads.wav"),
        QStringLiteral("Room Ambience.wav"),
        QStringLiteral("Reference Mix.wav"),
    };
}

QStringList MixerApp::defaultRecentProjects() const
{
    return {
        QStringLiteral("Studio Demo Session"),
        QStringLiteral("Podcast Voice Cleanup"),
        QStringLiteral("Band Rehearsal Mix"),
    };
}

QStringList MixerApp::filterAssets(const QStringList &catalog, const QString &query) const
{
    if (query.trimmed().isEmpty()) {
        return catalog;
    }

    QStringList filtered;
    for (const QString &name : catalog) {
        if (name.contains(query, Qt::CaseInsensitive)) {
            filtered.append(name);
        }
    }
    return filtered;
}

QString MixerApp::importMockTrack(int existingTrackCount)
{
    const QString name = QStringLiteral("Track %1").arg(existingTrackCount + 1);
    if (m_engine) {
        m_engine->importTrack(name);
    }
    return name;
}

bool MixerApp::importAssetByName(const QString &name, QString *statusOut)
{
    if (name.trimmed().isEmpty()) {
        if (statusOut) {
            *statusOut = QStringLiteral("Select an asset before importing.");
        }
        return false;
    }

    if (m_engine) {
        m_engine->importTrack(name);
    }
    if (statusOut) {
        *statusOut = QStringLiteral("Imported asset: %1").arg(name);
    }
    return true;
}

QStringList MixerApp::saveMockSnapshot(int trackCount, QStringList recent, QString *statusOut)
{
    if (trackCount <= 0) {
        if (statusOut) {
            *statusOut = QStringLiteral("Add at least one track before saving a project snapshot.");
        }
        return recent;
    }

    const QString name = QStringLiteral("Mock Project %1 tracks").arg(trackCount);
    recent.removeAll(name);
    recent.prepend(name);
    while (recent.size() > 5) {
        recent.removeLast();
    }

    if (statusOut) {
        *statusOut = QStringLiteral("Project snapshot queued: %1").arg(name);
    }
    return recent;
}

QString MixerApp::restoreRecentProject(const QString &name, QString *statusOut)
{
    if (name.trimmed().isEmpty()) {
        if (statusOut) {
            *statusOut = QStringLiteral("Select a recent project before restore.");
        }
        return {};
    }

    if (statusOut) {
        *statusOut = QStringLiteral("Restore project queued: %1").arg(name);
    }
    return name;
}
