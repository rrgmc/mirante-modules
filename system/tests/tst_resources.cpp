// The readings of the CPU, the memory and the disks. The values change with
// the computer, so the tests check their shape and the arithmetic.
#include "../SystemResources.h"

#include <QTest>

namespace sys = mirante::modules::system;

class TestResources : public QObject
{
    Q_OBJECT

private slots:
    void cpuUsageIsTheBusyShare()
    {
        const auto share = sys::cpuUsage({100, 400}, {130, 500});
        QVERIFY(share.has_value());
        QCOMPARE(*share, 0.7);
    }

    void cpuUsageNeedsTimeToPass()
    {
        QVERIFY(!sys::cpuUsage({100, 400}, {100, 400}).has_value());
        QVERIFY(!sys::cpuUsage({100, 400}, {90, 500}).has_value());
        QVERIFY(!sys::cpuUsage({100, 400}, {130, 300}).has_value());
    }

    void cpuUsageStaysInRange()
    {
        // More idle time than total time, as rounding can give.
        QCOMPARE(*sys::cpuUsage({0, 0}, {120, 100}), 0.0);
    }

    void readsTheCpu()
    {
        const auto first = sys::readCpuTimes();
        QVERIFY(first.has_value());
        QVERIFY(first->total > 0);
        QVERIFY(first->idle <= first->total);
        // Some work, so the counters move.
        volatile quint64 sum = 0;
        for (quint64 i = 0; i < 50000000; ++i)
            sum = sum + i;
        QTest::qWait(200);
        const auto second = sys::readCpuTimes();
        QVERIFY(second.has_value());
        QVERIFY(second->total >= first->total);
        if (const auto share = sys::cpuUsage(*first, *second)) {
            QVERIFY(*share >= 0);
            QVERIFY(*share <= 1);
        }
    }

    void readsTheMemory()
    {
        const auto memory = sys::readMemory();
        QVERIFY(memory.has_value());
        QVERIFY(memory->total > 0);
        QVERIFY(memory->available <= memory->total);
    }

    void listsTheLocalDisks()
    {
        const QStringList roots = sys::localDiskRoots();
        QVERIFY(!roots.isEmpty());
        for (const QString &root : roots)
            QVERIFY2(!root.contains(QLatin1Char('\\')), qPrintable(root));
    }

    void readsAllDisksForAnEmptyList()
    {
        const QList<sys::DiskInfo> disks = sys::readDisks({});
        QCOMPARE(disks.size(), sys::localDiskRoots().size());
        for (const sys::DiskInfo &disk : disks) {
            QVERIFY(disk.total > 0);
            QVERIFY(disk.free <= disk.total);
        }
    }

    void leavesOutRootsNotMounted()
    {
        const QString first = sys::localDiskRoots().value(0);
        const QList<sys::DiskInfo> disks =
            sys::readDisks({QStringLiteral("  "), QStringLiteral("/no/such/mirante/folder/xyz"), first});
        // QStorageInfo takes a missing folder as the volume above it, if any,
        // so only the blank entry is sure to go.
        QVERIFY(!disks.isEmpty());
        QCOMPARE(disks.last().root, first);
    }

    void usedShare()
    {
        QCOMPARE(sys::usedShare(0, 0), 0.0);
        QCOMPARE(sys::usedShare(200, 50), 0.75);
        QCOMPARE(sys::usedShare(200, 300), 0.0);
    }
};

QTEST_MAIN(TestResources)
#include "tst_resources.moc"
