#include <Model/ProjectStore.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonValue>

namespace {

QJsonArray eqBandsToJson(const AudioTrack &track)
{
    QJsonArray bands;
    const QVector<float> values = track.eqBandsOrDefault();
    for (float value : values) {
        bands.append(static_cast<double>(value));
    }
    return bands;
}

QVector<float> eqBandsFromJson(const QJsonObject &item)
{
    QVector<float> bands(AudioTrack::kEqBandCount, 0.0f);
    if (item.contains(QStringLiteral("eqBands")) && item.value(QStringLiteral("eqBands")).isArray()) {
        const QJsonArray array = item.value(QStringLiteral("eqBands")).toArray();
        for (int i = 0; i < AudioTrack::kEqBandCount; ++i) {
            bands[i] = i < array.size() ? static_cast<float>(array.at(i).toDouble(0.0)) : 0.0f;
        }
        return bands;
    }

    bands[1] = static_cast<float>(item.value(QStringLiteral("eqLowDb")).toDouble(0.0));
    bands[4] = static_cast<float>(item.value(QStringLiteral("eqMidDb")).toDouble(0.0));
    bands[7] = static_cast<float>(item.value(QStringLiteral("eqHighDb")).toDouble(0.0));
    return bands;
}

QJsonArray automationToJson(const QVector<AutomationPoint> &points)
{
    QJsonArray array;
    for (const AutomationPoint &point : points) {
        QJsonObject item;
        item.insert(QStringLiteral("timeMs"), point.timeMs);
        item.insert(QStringLiteral("value"), static_cast<double>(point.value));
        array.append(item);
    }
    return array;
}

QVector<AutomationPoint> automationFromJson(const QJsonObject &item)
{
    QVector<AutomationPoint> points;
    if (!item.contains(QStringLiteral("volumeAutomation"))
        || !item.value(QStringLiteral("volumeAutomation")).isArray()) {
        return points;
    }

    const QJsonArray array = item.value(QStringLiteral("volumeAutomation")).toArray();
    for (const QJsonValue &value : array) {
        if (!value.isObject()) {
            continue;
        }
        const QJsonObject pointObj = value.toObject();
        AutomationPoint point;
        point.timeMs = pointObj.value(QStringLiteral("timeMs")).toInt(0);
        point.value = static_cast<float>(pointObj.value(QStringLiteral("value")).toDouble(1.0));
        points.append(point);
    }
    return points;
}

} // namespace

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
        item.insert(QStringLiteral("playbackRate"), static_cast<double>(track.playbackRate));
        item.insert(QStringLiteral("muted"), track.muted);
        item.insert(QStringLiteral("solo"), track.solo);
        item.insert(QStringLiteral("eqLowDb"), static_cast<double>(track.eqLowDb));
        item.insert(QStringLiteral("eqMidDb"), static_cast<double>(track.eqMidDb));
        item.insert(QStringLiteral("eqHighDb"), static_cast<double>(track.eqHighDb));
        item.insert(QStringLiteral("eqBands"), eqBandsToJson(track));
        item.insert(QStringLiteral("compThreshold"), static_cast<double>(track.compThreshold));
        item.insert(QStringLiteral("compRatio"), static_cast<double>(track.compRatio));
        item.insert(QStringLiteral("fxBypass"), track.fxBypass);
        item.insert(QStringLiteral("loopEnabled"), track.loopEnabled);
        item.insert(QStringLiteral("loopStart"), static_cast<double>(track.loopStart));
        item.insert(QStringLiteral("loopEnd"), static_cast<double>(track.loopEnd));
        item.insert(QStringLiteral("volumeAutomation"), automationToJson(track.volumeAutomation));
        tracks.append(item);
    }
    root.insert(QStringLiteral("tracks"), tracks);
    return root;
}

bool ProjectStore::applyJson(AudioEngine &engine,
                             const QJsonObject &root,
                             QString *errorMessage,
                             const QString &baseDir)
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
        QString path = item.value(QStringLiteral("sourcePath")).toString();
        if (path.isEmpty()) {
            path = item.value(QStringLiteral("displayName")).toString();
        }
        if (!path.isEmpty() && !QFileInfo(path).isAbsolute() && !baseDir.isEmpty()) {
            path = QDir(baseDir).filePath(path);
        }
        engine.importTrack(path);

        const int index = engine.trackCount() - 1;
        engine.setTrackVolume(index, static_cast<float>(item.value(QStringLiteral("volume")).toDouble(0.8)));
        engine.setTrackPan(index, static_cast<float>(item.value(QStringLiteral("pan")).toDouble(0.0)));
        engine.setTrackPlaybackRate(index, static_cast<float>(item.value(QStringLiteral("playbackRate")).toDouble(1.0)));
        engine.setTrackMuted(index, item.value(QStringLiteral("muted")).toBool(false));
        engine.setTrackSolo(index, item.value(QStringLiteral("solo")).toBool(false));
        engine.setTrackEqBands(index, eqBandsFromJson(item));
        engine.setTrackCompressor(index,
                                  static_cast<float>(item.value(QStringLiteral("compThreshold")).toDouble(0.7)),
                                  static_cast<float>(item.value(QStringLiteral("compRatio")).toDouble(4.0)));
        engine.setTrackFxBypass(index, item.value(QStringLiteral("fxBypass")).toBool(false));
        engine.setTrackLoopEnabled(index, item.value(QStringLiteral("loopEnabled")).toBool(false));
        engine.setTrackLoopRange(index,
                                 static_cast<float>(item.value(QStringLiteral("loopStart")).toDouble(0.0)),
                                 static_cast<float>(item.value(QStringLiteral("loopEnd")).toDouble(1.0)));
        engine.setTrackVolumeAutomation(index, automationFromJson(item));
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

bool ProjectStore::loadFromFile(AudioEngine &engine,
                                const QString &path,
                                QString *errorMessage,
                                const QString &baseDir)
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

    const QString resolvedBase = baseDir.isEmpty() ? QFileInfo(path).absolutePath() : baseDir;
    return applyJson(engine, document.object(), errorMessage, resolvedBase);
}
