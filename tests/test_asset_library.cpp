#include <Model/AssetLibrary.h>

#include <QCoreApplication>
#include <QTemporaryDir>

#include <cstdlib>
#include <iostream>

namespace {

void expectTrue(bool condition, const char *message)
{
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QTemporaryDir temp;
    expectTrue(temp.isValid(), "Temp dir should be valid");
    const QString dbPath = temp.filePath(QStringLiteral("assets.sqlite"));

    AssetLibrary library(QStringLiteral("test_assets_conn"));
    QString error;
    expectTrue(library.open(dbPath, &error), "Open SQLite library should succeed");
    expectTrue(library.upsertAsset(QStringLiteral("C:/audio/Kick.wav"), QStringLiteral("Kick.wav"), &error),
               "Upsert kick should succeed");
    expectTrue(library.upsertAsset(QStringLiteral("C:/audio/Snare.wav"), QStringLiteral("Snare.wav"), &error),
               "Upsert snare should succeed");

    const QStringList all = library.search(QString());
    expectTrue(all.size() == 2, "Empty search should list all assets");

    const QStringList kicks = library.search(QStringLiteral("Kick"));
    expectTrue(kicks.size() == 1, "Search Kick should return one result");
    expectTrue(kicks.at(0) == QStringLiteral("Kick.wav"), "Search should return Kick.wav");

    const QStringList recent = library.recent(1);
    expectTrue(recent.size() == 1, "Recent limit 1 should return one item");

    return 0;
}
