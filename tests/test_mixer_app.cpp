#include <App/MixerApp.h>

#include <QCoreApplication>
#include <cstdio>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Q_UNUSED(app);

    int failures = 0;

    {
        const SoloPlan empty = MixerApp::planSolo({});
        if (empty.anySolo || !empty.blockedBySolo.isEmpty()) {
            std::fprintf(stderr, "planSolo(empty) failed\n");
            ++failures;
        }
    }

    {
        const SoloPlan none = MixerApp::planSolo({false, false, false});
        if (none.anySolo || none.blockedBySolo != QVector<bool>({false, false, false})) {
            std::fprintf(stderr, "planSolo(no solo) failed\n");
            ++failures;
        }
    }

    {
        const SoloPlan one = MixerApp::planSolo({false, true, false});
        if (!one.anySolo || one.blockedBySolo != QVector<bool>({true, false, true})) {
            std::fprintf(stderr, "planSolo(one solo) failed\n");
            ++failures;
        }
    }

    if (failures == 0) {
        std::printf("test_mixer_app: OK\n");
        return 0;
    }

    std::fprintf(stderr, "test_mixer_app: %d failure(s)\n", failures);
    return 1;
}
