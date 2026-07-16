#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

#include <algorithm>

struct TrackDspParams
{
    float volume = 0.8f;
    float pan = 0.0f;
    float playbackRate = 1.0f;
    bool muted = false;
    bool solo = false;
    float eqLowDb = 0.0f;
    float eqMidDb = 0.0f;
    float eqHighDb = 0.0f;
    QVector<float> eqBands;
    float compThreshold = 0.7f;
    float compRatio = 4.0f;
    bool fxBypass = false;
    bool loopEnabled = false;
    float loopStart = 0.0f;
    float loopEnd = 1.0f;
};

struct SoloPlan
{
    bool anySolo = false;
    QVector<bool> blockedBySolo;
};

inline SoloPlan planSolo(const QVector<bool> &soloFlags)
{
    SoloPlan plan;
    plan.anySolo = std::any_of(soloFlags.cbegin(), soloFlags.cend(), [](bool solo) {
        return solo;
    });
    plan.blockedBySolo.reserve(soloFlags.size());
    for (bool solo : soloFlags) {
        plan.blockedBySolo.append(plan.anySolo && !solo);
    }
    return plan;
}

inline QStringList defaultAssetCatalog()
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

inline QStringList defaultRecentProjects()
{
    return {
        QStringLiteral("Studio Demo Session"),
        QStringLiteral("Podcast Voice Cleanup"),
        QStringLiteral("Band Rehearsal Mix"),
    };
}

inline QStringList filterAssets(const QStringList &catalog, const QString &query)
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

inline QStringList saveMockSnapshot(int trackCount, QStringList recent, QString *statusOut)
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

inline QString restoreRecentProjectStatus(const QString &name, QString *statusOut)
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
