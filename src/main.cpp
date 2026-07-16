#include <App/MixingStudioApp.h>

#include <QQuickStyle>

int main(int argc, char *argv[])
{
    // Before QGuiApplication: force Material so Controls never fall back to light Basic/Fusion.
    qputenv("QT_QUICK_CONTROLS_STYLE", "Material");
    qputenv("QT_QUICK_CONTROLS_MATERIAL_THEME", "Dark");
    QQuickStyle::setStyle(QStringLiteral("Material"));

    MixingStudioApp app;
    return app.run(argc, argv);
}
