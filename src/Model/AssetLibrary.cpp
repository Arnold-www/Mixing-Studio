#include <Model/AssetLibrary.h>

#include <algorithm>
#include <utility>

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

AssetLibrary::AssetLibrary(QString connectionName)
    : m_connectionName(std::move(connectionName))
{
}

AssetLibrary::~AssetLibrary()
{
    close();
}

bool AssetLibrary::open(const QString &databasePath, QString *errorMessage)
{
    close();

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    db.setDatabaseName(databasePath);
    if (!db.open()) {
        if (errorMessage) {
            *errorMessage = db.lastError().text();
        }
        QSqlDatabase::removeDatabase(m_connectionName);
        return false;
    }

    m_open = true;
    if (!ensureSchema(errorMessage)) {
        close();
        return false;
    }
    return true;
}

void AssetLibrary::close()
{
    if (!m_open) {
        return;
    }

    {
        QSqlDatabase db = database();
        if (db.isOpen()) {
            db.close();
        }
    }
    QSqlDatabase::removeDatabase(m_connectionName);
    m_open = false;
}

bool AssetLibrary::isOpen() const
{
    return m_open;
}

bool AssetLibrary::ensureSchema(QString *errorMessage)
{
    QSqlQuery query(database());
    const bool ok = query.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS assets ("
        "  path TEXT PRIMARY KEY,"
        "  name TEXT NOT NULL,"
        "  added_at INTEGER NOT NULL"
        ")"));
    if (!ok && errorMessage) {
        *errorMessage = query.lastError().text();
    }
    return ok;
}

QSqlDatabase AssetLibrary::database() const
{
    return QSqlDatabase::database(m_connectionName);
}

bool AssetLibrary::upsertAsset(const QString &path, const QString &displayName, QString *errorMessage)
{
    if (!m_open) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Asset library is not open.");
        }
        return false;
    }

    QSqlQuery query(database());
    query.prepare(QStringLiteral(
        "INSERT INTO assets(path, name, added_at) VALUES(?, ?, strftime('%s','now')) "
        "ON CONFLICT(path) DO UPDATE SET name=excluded.name, added_at=strftime('%s','now')"));
    query.addBindValue(path);
    query.addBindValue(displayName.isEmpty() ? path : displayName);
    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return true;
}

QStringList AssetLibrary::search(const QString &queryText) const
{
    QStringList results;
    if (!m_open) {
        return results;
    }

    QSqlQuery query(database());
    if (queryText.trimmed().isEmpty()) {
        query.exec(QStringLiteral("SELECT name FROM assets ORDER BY added_at DESC"));
    } else {
        query.prepare(QStringLiteral(
            "SELECT name FROM assets WHERE name LIKE ? OR path LIKE ? ORDER BY added_at DESC"));
        const QString pattern = QStringLiteral("%") + queryText.trimmed() + QStringLiteral("%");
        query.addBindValue(pattern);
        query.addBindValue(pattern);
        query.exec();
    }

    while (query.next()) {
        results.append(query.value(0).toString());
    }
    return results;
}

QStringList AssetLibrary::recent(int limit) const
{
    QStringList results;
    if (!m_open) {
        return results;
    }

    QSqlQuery query(database());
    query.prepare(QStringLiteral("SELECT name FROM assets ORDER BY added_at DESC LIMIT ?"));
    query.addBindValue(std::max(1, limit));
    if (!query.exec()) {
        return results;
    }

    while (query.next()) {
        results.append(query.value(0).toString());
    }
    return results;
}
