#include <App/MixingStudioApp.h>

#include <Model/AudioEngine.h>
#include <ViewModel/IMixerViewModel.h>
#include <ViewModel/RealMixerViewModel.h>

#include <QColor>
#include <QPalette>
#include <QQmlContext>
#include <QQuickStyle>

MixingStudioApp::MixingStudioApp() = default;

MixingStudioApp::~MixingStudioApp()
{
    // Owned by Qt parent chain / stack order in run(); nothing extra when run() owns locals.
}

void MixingStudioApp::bindViewModelToQml(QQmlApplicationEngine &engine)
{
    // Interface pointer only — QML must not depend on RealMixerViewModel.
    engine.rootContext()->setContextProperty(QStringLiteral("mixerViewModel"), m_viewModel);
}

namespace {

void applyNightPalette(QGuiApplication &guiApp)
{
    QPalette pal = guiApp.palette();
    const QColor windowBg(0x0b, 0x0f, 0x14);
    const QColor panel(0x14, 0x1b, 0x24);
    const QColor panelAlt(0x1a, 0x23, 0x30);
    const QColor text(0xe8, 0xee, 0xf6);
    const QColor muted(0x9a, 0xa8, 0xb8);
    const QColor button(0x2a, 0x35, 0x45);
    const QColor accent(0x3d, 0x9b, 0x84);
    const QColor mid(0x2c, 0x3a, 0x4d);

    pal.setColor(QPalette::Window, windowBg);
    pal.setColor(QPalette::WindowText, text);
    pal.setColor(QPalette::Base, panelAlt);
    pal.setColor(QPalette::AlternateBase, panel);
    pal.setColor(QPalette::Text, text);
    pal.setColor(QPalette::Button, button);
    pal.setColor(QPalette::ButtonText, text);
    pal.setColor(QPalette::BrightText, text);
    pal.setColor(QPalette::ToolTipBase, panel);
    pal.setColor(QPalette::ToolTipText, text);
    pal.setColor(QPalette::Highlight, accent);
    pal.setColor(QPalette::HighlightedText, Qt::white);
    pal.setColor(QPalette::Light, mid);
    pal.setColor(QPalette::Mid, mid);
    pal.setColor(QPalette::Dark, windowBg);
    pal.setColor(QPalette::Shadow, Qt::black);
    pal.setColor(QPalette::PlaceholderText, muted);
    guiApp.setPalette(pal);
}

} // namespace

int MixingStudioApp::run(int argc, char *argv[])
{
    qputenv("QT_QUICK_CONTROLS_STYLE", "Material");
    qputenv("QT_QUICK_CONTROLS_MATERIAL_THEME", "Dark");
    QQuickStyle::setStyle(QStringLiteral("Material"));

    QGuiApplication guiApp(argc, argv);
    m_guiApp = &guiApp;
    applyNightPalette(guiApp);

    AudioEngine audioEngine;
    m_engine = &audioEngine;

    RealMixerViewModel realViewModel(&audioEngine);
    m_realViewModel = &realViewModel;
    m_viewModel = &realViewModel;

    QQmlApplicationEngine engine;
    bindViewModelToQml(engine);
    engine.loadFromModule("MixingStudio", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return guiApp.exec();
}
