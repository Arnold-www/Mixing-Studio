#include <App/MixingStudioApp.h>
#include <Model/AudioEngine.h>
#include <ViewModel/RealMixerViewModel.h>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTest>
#include <QTimer>
#include <QUrl>
#include <QVariant>
#include <QVariantList>
#include <QtQml/qqmlextensionplugin.h>

#include <cmath>
#include <cstdio>

Q_IMPORT_QML_PLUGIN(MixingStudioPlugin)

#ifndef MIXINGSTUDIO_SOURCE_DIR
#  define MIXINGSTUDIO_SOURCE_DIR ""
#endif

namespace {

int g_failures = 0;
bool g_headless = false;

void expectTrue(bool cond, const char *msg)
{
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", msg);
        ++g_failures;
    }
}

void flushUi(int rounds = 12)
{
    for (int i = 0; i < rounds; ++i) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 40);
    }
}

void stepPause()
{
    // Visible runs: brief pause so each action is observable. Headless: short.
    const int defaultMs = g_headless ? 80 : 450;
    const int ms = qEnvironmentVariableIsSet("MIXINGSTUDIO_UI_STEP_MS")
        ? qEnvironmentVariableIntValue("MIXINGSTUDIO_UI_STEP_MS")
        : defaultMs;
    if (ms > 0)
        QTest::qWait(ms);
}

void logStep(const char *msg)
{
    std::printf("  [%s]\n", msg);
    std::fflush(stdout);
}

QQuickItem *requireItem(QObject *root, const char *objectName)
{
    auto *item = root->findChild<QQuickItem *>(QLatin1String(objectName), Qt::FindChildrenRecursively);
    if (!item) {
        std::fprintf(stderr, "FAIL: missing Quick item objectName=%s\n", objectName);
        ++g_failures;
    }
    return item;
}

QPoint centerInWindow(QQuickItem *item)
{
    const QPointF local(item->width() * 0.5, item->height() * 0.5);
    return item->mapToScene(local).toPoint();
}

void mouseClickItem(QQuickItem *item)
{
    if (!item)
        return;
    QQuickWindow *window = item->window();
    expectTrue(window != nullptr, "item has QQuickWindow for mouseClick");
    if (!window)
        return;
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, centerInWindow(item));
    flushUi(4);
}

void mouseClickAtFraction(QQuickItem *item, qreal fx, qreal fy)
{
    if (!item)
        return;
    QQuickWindow *window = item->window();
    expectTrue(window != nullptr, "item has QQuickWindow for fractional click");
    if (!window)
        return;
    const QPointF local(item->width() * fx, item->height() * fy);
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, item->mapToScene(local).toPoint());
    flushUi(4);
}

void typeIntoField(QQuickItem *field, const QString &text)
{
    if (!field)
        return;
    QQuickWindow *window = field->window();
    expectTrue(window != nullptr, "field has QQuickWindow for keyboard");
    if (!window)
        return;

    mouseClickItem(field);
    field->forceActiveFocus();
    flushUi(4);
    QTest::keyClick(window, Qt::Key_A, Qt::ControlModifier);
    flushUi(2);
    for (const QChar ch : text) {
        QTest::keyClick(window, ch.toLatin1());
    }
    flushUi(2);
    QTest::keyClick(window, Qt::Key_Return);
    flushUi(6);
}

QString resolveDemoToneWav()
{
    const QString rel = QStringLiteral("samples/demo_tone.wav");
    const QStringList candidates = {
        QDir(QString::fromUtf8(MIXINGSTUDIO_SOURCE_DIR)).filePath(rel),
        QDir::current().absoluteFilePath(rel),
        QDir(QCoreApplication::applicationDirPath()).filePath(rel),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../") + rel),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../../") + rel),
    };
    for (const QString &path : candidates) {
        if (!path.isEmpty() && QFileInfo::exists(path))
            return QFileInfo(path).absoluteFilePath();
    }
    return {};
}

bool importWavViaUiSignal(QObject *root, const QString &localPath)
{
    QVariantList urls;
    urls.append(QUrl::fromLocalFile(localPath).toString());
    return QMetaObject::invokeMethod(root,
                                     "importFilesRequested",
                                     Qt::DirectConnection,
                                     Q_ARG(QVariant, QVariant::fromValue(urls)));
}

} // namespace

int main(int argc, char *argv[])
{
    g_headless = qEnvironmentVariableIsSet("CI")
        || qEnvironmentVariableIntValue("MIXINGSTUDIO_UI_HEADLESS") != 0;
    if (g_headless) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }

    qputenv("QT_QUICK_CONTROLS_STYLE", "Material");
    qputenv("QT_QUICK_CONTROLS_MATERIAL_THEME", "Dark");
    QQuickStyle::setStyle(QStringLiteral("Material"));

    QGuiApplication app(argc, argv);

    AudioEngine audioEngine;
    RealMixerViewModel viewModel(&audioEngine);

    QQmlApplicationEngine engine;
    engine.loadFromModule("MixingStudio", "Main");
    if (engine.rootObjects().isEmpty()) {
        std::fprintf(stderr, "FAIL: loadFromModule(Main) returned no root\n");
        return 1;
    }

    QObject *root = engine.rootObjects().constFirst();
    expectTrue(root->objectName() == QLatin1String("mainWindow"), "root objectName is mainWindow");

    auto *window = qobject_cast<QQuickWindow *>(root);
    expectTrue(window != nullptr, "root is QQuickWindow");
    if (!window)
        return 1;

    MixingStudioViewBinder binder(&viewModel);
    binder.bind(root);

    window->show();
    expectTrue(QTest::qWaitForWindowExposed(window, 5000), "main window exposed");
    flushUi();
    std::printf("ui_binder demo: window open (%s)\n", g_headless ? "headless" : "visible");

    // --- 1) Import demo_tone.wav ---
    logStep("1 import demo_tone.wav");
    const QString wavPath = resolveDemoToneWav();
    expectTrue(!wavPath.isEmpty(), "samples/demo_tone.wav found");
    if (wavPath.isEmpty())
        return 1;
    std::printf("      path=%s\n", qPrintable(wavPath));
    expectTrue(importWavViaUiSignal(root, wavPath), "importFilesRequested(demo_tone.wav)");
    flushUi(20);
    stepPause();
    expectTrue(!root->property("tracks").toList().isEmpty(), "track present after import");

    // --- 2) Demo track (extra strip in mixer) ---
    logStep("2 click Demo (add mock track)");
    mouseClickItem(requireItem(root, "demoButton"));
    flushUi(12);
    stepPause();
    expectTrue(root->property("tracks").toList().size() >= 2, "at least 2 tracks after Demo");

    // --- 3) Open Spectrum panel ---
    logStep("3 open Spectrum panel");
    mouseClickItem(requireItem(root, "spectrumButton"));
    flushUi(8);
    stepPause();

    // --- 4) Open Library drawer briefly ---
    logStep("4 toggle Library drawer");
    mouseClickItem(requireItem(root, "libraryButton"));
    flushUi(8);
    stepPause();
    mouseClickItem(requireItem(root, "libraryButton"));
    flushUi(6);
    stepPause();

    // --- 5) Master volume keyboard (70%) ---
    logStep("5 type Master volume 70%");
    typeIntoField(requireItem(root, "masterVolumeField"), QStringLiteral("70"));
    flushUi(10);
    stepPause();
    {
        const float mv = root->property("masterVolume").toFloat();
        expectTrue(std::fabs(mv - 0.70f) < 0.03f, "masterVolume ~0.70 after keyboard");
    }

    // --- 6) Song Loop on ---
    logStep("6 enable Song Loop");
    mouseClickItem(requireItem(root, "songLoopButton"));
    flushUi(8);
    stepPause();
    expectTrue(root->property("loopEnabled").toBool() == true, "loopEnabled true");

    // --- 7) Seek ~35% ---
    logStep("7 seek timeline ~35%");
    mouseClickAtFraction(requireItem(root, "seekArea"), 0.35, 0.50);
    flushUi(8);
    stepPause();
    expectTrue(root->property("playbackProgress").toFloat() >= 0.0f, "playbackProgress readable");

    // --- 8) Play ---
    logStep("8 Play");
    mouseClickItem(requireItem(root, "playPauseButton"));
    flushUi(10);
    stepPause();
    expectTrue(root->property("playing").toBool() == true, "playing true after Play");

    // --- 9) Hold play (loop + meters) ---
    const int playMs = qEnvironmentVariableIsSet("MIXINGSTUDIO_UI_PLAY_MS")
        ? qEnvironmentVariableIntValue("MIXINGSTUDIO_UI_PLAY_MS")
        : (g_headless ? 3000 : 8000);
    logStep("9 playing with loop");
    std::printf("      duration=%d ms\n", playMs);
    QTest::qWait(playMs);
    expectTrue(root->property("playing").toBool() == true, "still playing after wait");
    expectTrue(root->property("loopEnabled").toBool() == true, "loop still on");

    // --- 10) Pause then Play again ---
    logStep("10 Pause");
    mouseClickItem(requireItem(root, "playPauseButton"));
    flushUi(8);
    stepPause();
    expectTrue(root->property("playing").toBool() == false, "paused");

    logStep("11 Play again");
    mouseClickItem(requireItem(root, "playPauseButton"));
    flushUi(8);
    stepPause();
    expectTrue(root->property("playing").toBool() == true, "playing again");
    QTest::qWait(g_headless ? 400 : 1200);

    // --- 12) Stop ---
    logStep("12 Stop");
    mouseClickItem(requireItem(root, "stopButton"));
    flushUi(8);
    stepPause();
    expectTrue(root->property("playing").toBool() == false, "stopped");

    // --- 13) Save project ---
    logStep("13 Save project");
    mouseClickItem(requireItem(root, "saveButton"));
    flushUi(16);
    stepPause();
    expectTrue(!root->property("statusMessage").toString().isEmpty(), "status after Save");

    // --- 14) Mock toggle (analysis) ---
    logStep("14 toggle Mock analysis");
    mouseClickItem(requireItem(root, "mockButton"));
    flushUi(10);
    stepPause();
    expectTrue(root->property("mockValidationMode").toBool() == true, "mock mode on");
    mouseClickItem(requireItem(root, "mockButton"));
    flushUi(8);
    stepPause();

    // --- 15) Projects drawer glance ---
    logStep("15 toggle Projects drawer");
    mouseClickItem(requireItem(root, "projectsButton"));
    flushUi(8);
    stepPause();
    mouseClickItem(requireItem(root, "projectsButton"));
    flushUi(6);
    stepPause();

    // --- 16) Close ---
    logStep("16 close window");
    window->close();
    QTimer::singleShot(0, &app, &QGuiApplication::quit);
    app.exec();

    if (g_failures == 0) {
        std::printf("test_ui_binder: OK (expanded demo, play %dms)\n", playMs);
        return 0;
    }
    std::fprintf(stderr, "test_ui_binder: %d failure(s)\n", g_failures);
    return 1;
}
