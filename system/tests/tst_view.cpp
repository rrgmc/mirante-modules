// The System Resources view, made from a configuration. It reads the real
// computer, so the tests check which rows it shows.
#include "../ResourcesView.h"
#include "../SystemResources.h"

#include <mirante/testing/FakeItemContext.h>

#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QTest>
#include <QTimer>

#include <memory>

namespace sys = mirante::modules::system;

namespace {

QStringList rowKeys(const QWidget &view)
{
    QStringList keys;
    for (const QLabel *label : view.findChildren<QLabel *>()) {
        if (label->objectName().startsWith(QLatin1String("value.")))
            keys << label->objectName().mid(6);
    }
    keys.sort();
    return keys;
}

QString valueOf(const QWidget &view, const QString &key)
{
    const auto *label = view.findChild<QLabel *>(QStringLiteral("value.") + key);
    return label ? label->text() : QString();
}

} // namespace

class TestView : public QObject
{
    Q_OBJECT

private slots:
    void showsEverythingByDefault()
    {
        mirante::testing::FakeItemContext context;
        std::unique_ptr<QWidget> view(sys::createResourcesView(context, nullptr));
        QVERIFY(view);

        QStringList expected{QStringLiteral("cpu"), QStringLiteral("memory")};
        for (const QString &root : sys::localDiskRoots())
            expected << QStringLiteral("disk:") + root;
        expected.sort();
        QCOMPARE(rowKeys(*view), expected);
        QVERIFY(valueOf(*view, QStringLiteral("memory")).contains(QStringLiteral("free of")));
        QVERIFY(!view->findChild<QLabel *>(QStringLiteral("empty"))->isVisibleTo(view.get()));
    }

    void cpuHasAValueAfterASecondReading()
    {
        mirante::testing::FakeItemContext context;
        sys::ResourcesView view(context.config());
        QCOMPARE(valueOf(view, QStringLiteral("cpu")), QStringLiteral("Measuring…"));
        QTest::qWait(300);
        view.refresh();
        QVERIFY2(valueOf(view, QStringLiteral("cpu")).endsWith(QStringLiteral("%")),
                 qPrintable(valueOf(view, QStringLiteral("cpu"))));
    }

    void showsOnlyWhatIsChosen()
    {
        mirante::testing::FakeItemContext context;
        context.itemConfig = QJsonObject{
            {QStringLiteral("showCpu"), false},
            {QStringLiteral("showMemory"), true},
            {QStringLiteral("showDisks"), false},
        };
        std::unique_ptr<QWidget> view(sys::createResourcesView(context, nullptr));
        QCOMPARE(rowKeys(*view), QStringList{QStringLiteral("memory")});
    }

    void showsTheChosenDisks()
    {
        const QString root = sys::localDiskRoots().value(0);
        mirante::testing::FakeItemContext context;
        context.itemConfig = QJsonObject{
            {QStringLiteral("showCpu"), false},
            {QStringLiteral("showMemory"), false},
            {QStringLiteral("disks"), QJsonArray{root, QString()}},
        };
        std::unique_ptr<QWidget> view(sys::createResourcesView(context, nullptr));
        QCOMPARE(rowKeys(*view), QStringList{QStringLiteral("disk:") + root});
        QVERIFY(valueOf(*view, QStringLiteral("disk:") + root).contains(QStringLiteral("free of")));
    }

    void saysWhenNothingIsChosen()
    {
        mirante::testing::FakeItemContext context;
        context.itemConfig = QJsonObject{
            {QStringLiteral("showCpu"), false},
            {QStringLiteral("showMemory"), false},
            {QStringLiteral("showDisks"), false},
        };
        std::unique_ptr<QWidget> view(sys::createResourcesView(context, nullptr));
        QVERIFY(rowKeys(*view).isEmpty());
        QVERIFY(view->findChild<QLabel *>(QStringLiteral("empty"))->isVisibleTo(view.get()));
    }

    void refreshesOnlyWhileVisible()
    {
        mirante::testing::FakeItemContext context;
        context.itemConfig = QJsonObject{{QStringLiteral("interval"), 5}};
        sys::ResourcesView view(context.config());
        QCOMPARE(view.intervalMs(), 5000);
        auto *timer = view.findChild<QTimer *>(QStringLiteral("refresh"));
        QVERIFY(!timer->isActive());
        view.show();
        QVERIFY(timer->isActive());
        view.hide();
        QVERIFY(!timer->isActive());
    }

    void clampsTheInterval()
    {
        QCOMPARE(sys::ResourcesView(QJsonObject{{QStringLiteral("interval"), 0}}).intervalMs(), 1000);
        QCOMPARE(sys::ResourcesView(QJsonObject{{QStringLiteral("interval"), 99999}}).intervalMs(), 3600000);
    }
};

QTEST_MAIN(TestView)
#include "tst_view.moc"
