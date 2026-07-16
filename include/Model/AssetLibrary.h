#pragma once

#include <QSqlDatabase>
#include <QString>
#include <QStringList>

class AssetLibrary
{
public:
    explicit AssetLibrary(QString connectionName = QStringLiteral("mixing_studio_assets"));
    ~AssetLibrary();

    bool open(const QString &databasePath, QString *errorMessage = nullptr);
    void close();
    bool isOpen() const;

    bool upsertAsset(const QString &path, const QString &displayName, QString *errorMessage = nullptr);
    QStringList search(const QString &query) const;
    QStringList recent(int limit = 10) const;
    QString pathForName(const QString &displayName) const;

private:
    bool ensureSchema(QString *errorMessage);
    QSqlDatabase database() const;

    QString m_connectionName;
    bool m_open = false;
};
