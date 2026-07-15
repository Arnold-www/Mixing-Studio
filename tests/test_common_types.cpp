#include <Common/MixerTypes.h>

#include <QCoreApplication>
#include <cstdio>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Q_UNUSED(app);

    int failures = 0;

    {
        const SoloPlan empty = planSolo({});
        if (empty.anySolo || !empty.blockedBySolo.isEmpty()) {
            std::fprintf(stderr, "planSolo(empty) failed\n");
            ++failures;
        }
    }

    {
        const SoloPlan none = planSolo({false, false, false});
        if (none.anySolo || none.blockedBySolo != QVector<bool>({false, false, false})) {
            std::fprintf(stderr, "planSolo(no solo) failed\n");
            ++failures;
        }
    }

    {
        const SoloPlan one = planSolo({false, true, false});
        if (!one.anySolo || one.blockedBySolo != QVector<bool>({true, false, true})) {
            std::fprintf(stderr, "planSolo(one solo) failed\n");
            ++failures;
        }
    }

    if (failures == 0) {
        std::printf("test_common_types: OK\n");
        return 0;
    }

    std::fprintf(stderr, "test_common_types: %d failure(s)\n", failures);
    return 1;
}
