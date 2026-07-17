#pragma once

#include <Model/AudioEngine.h>

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

class ProjectStore
{
public:
    static constexpr int kSchemaVersion = 1;

    static QJsonObject toJson(const AudioEngine &engine);
    static bool applyJson(AudioEngine &engine,
                          const QJsonObject &root,
                          QString *errorMessage = nullptr,
                          const QString &baseDir = QString());
    static bool saveToFile(const AudioEngine &engine, const QString &path, QString *errorMessage = nullptr);
    static bool loadFromFile(AudioEngine &engine,
                             const QString &path,
                             QString *errorMessage = nullptr,
                             const QString &baseDir = QString());
};
