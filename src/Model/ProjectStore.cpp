#include <Model/ProjectStore.h>

#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonValue>

QJsonObject ProjectStore::toJson(const AudioEngine &engine)
{
    QJsonObject root;
    root.insert(QStringLiteral("version"), kSchemaVersion);
    root.insert(QStringLiteral("masterVolume"), static_cast<double>(engine.masterVolume()));
    root.insert(QStringLiteral("positionMs"), engine.positionMs());
    root.insert(QStringLiteral("durationMs"), engine.durationMs());
    root.insert(QStringLiteral("loopStartMs"), engine.loopStartMs());
    root.insert(QStringLiteral("loopEndMs"), engine.loopEndMs());

    QJsonArray tracks;
    const QList<AudioTrack> trackData = engine.trackData();
    for (const AudioTrack &track : trackData) {
        QJsonObject item;
        item.insert(QStringLiteral("sourcePath"), track.sourcePath);
        item.insert(QStringLiteral("displayName"), track.displayName);
        item.insert(QStringLiteral("volume"), static_cast<double>(track.volume));
        item.insert(QStringLiteral("pan"), static_cast<double>(track.pan));
        item.insert(QStringLiteral("muted"), track.muted);
        item.insert(QStringLiteral("solo"), track.solo);
        item.insert(QStringLiteral("eqLowDb"), static_cast<double>(track.eqLowDb));
        item.insert(QStringLiteral("eqMidDb"), static_cast<double>(track.eqMidDb));
        item.insert(QStringLiteral("eqHighDb"), static_cast<double>(track.eqHighDb));
        item.insert(QStringLiteral("compThreshold"), static_cast<double>(track.compThreshold));
        item.insert(QStringLiteral("compRatio"), static_cast<double>(track.compRatio));
        item.insert(QStringLiteral("fxBypass"), track.fxBypass);
        tracks.append(item);
    }
    root.insert(QStringLiteral("tracks"), tracks);
    return root;
}

bool ProjectStore::applyJson(AudioEngine &engine, const QJsonObject &root, QString *errorMessage)
{
    if (!root.contains(QStringLiteral("version")) || !root.contains(QStringLiteral("tracks"))) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid project JSON: missing version or tracks.");
        }
        return false;
    }

    const int version = root.value(QStringLiteral("version")).toInt();
    if (version != kSchemaVersion) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Unsupported project version: %1").arg(version);
        }
        return false;
    }

    engine.clearTracks();

    const QJsonArray tracks = root.value(QStringLiteral("tracks")).toArray();
    for (const QJsonValue &value : tracks) {
        if (!value.isObject()) {
            continue;
        }
        const QJsonObject item = value.toObject();
        const QString path = item.value(QStringLiteral("sourcePath")).toString();
        engine.importTrack(path.isEmpty() ? item.value(QStringLiteral("displayName")).toString() : path);

        const int index = engine.trackCount() - 1;
        engine.setTrackVolume(index, static_cast<float>(item.value(QStringLiteral("volume")).toDouble(0.8)));
        engine.setTrackPan(index, static_cast<float>(item.value(QStringLiteral("pan")).toDouble(0.0)));
        engine.setTrackMuted(index, item.value(QStringLiteral("muted")).toBool(false));
        engine.setTrackSolo(index, item.value(QStringLiteral("solo")).toBool(false));
        engine.setTrackEq(index,
                          static_cast<float>(item.value(QStringLiteral("eqLowDb")).toDouble(0.0)),
                          static_cast<float>(item.value(QStringLiteral("eqMidDb")).toDouble(0.0)),
                          static_cast<float>(item.value(QStringLiteral("eqHighDb")).toDouble(0.0)));
        engine.setTrackCompressor(index,
                                  static_cast<float>(item.value(QStringLiteral("compThreshold")).toDouble(0.7)),
                                  static_cast<float>(item.value(QStringLiteral("compRatio")).toDouble(4.0)));
        engine.setTrackFxBypass(index, item.value(QStringLiteral("fxBypass")).toBool(false));
    }

    engine.setMasterVolume(static_cast<float>(root.value(QStringLiteral("masterVolume")).toDouble(1.0)));
    engine.setLoopRange(root.value(QStringLiteral("loopStartMs")).toInt(0),
                        root.value(QStringLiteral("loopEndMs")).toInt(0));
    engine.seek(root.value(QStringLiteral("positionMs")).toInt(0));
    return true;
}

bool ProjectStore::saveToFile(const AudioEngine &engine, const QString &path, QString *errorMessage)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Cannot write project file: %1").arg(path);
        }
        return false;
    }

    const QJsonDocument document(toJson(engine));
    file.write(document.toJson(QJsonDocument::Indented));
    return true;
}

bool ProjectStore::loadFromFile(AudioEngine &engine, const QString &path, QString *errorMessage)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Cannot read project file: %1").arg(path);
        }
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid JSON: %1").arg(parseError.errorString());
        }
        return false;
    }

    return applyJson(engine, document.object(), errorMessage);
}
