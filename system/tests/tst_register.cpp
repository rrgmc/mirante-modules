// Registers the system module into a fresh registry and checks its type.
#include <mirante/testing/TestRegistrar.h>

#include <QIcon>
#include <QPixmap>
#include <QTest>

namespace mirante::modules::system {
void registerModule(mirante::ModuleRegistrar &registrar);
}

class TestRegister : public QObject
{
    Q_OBJECT

private slots:
    void registersTheType()
    {
        mirante::testing::TestRegistrar registry;
        registry.registerModule(QStringLiteral("system"), &mirante::modules::system::registerModule);

        QCOMPARE(registry.types().size(), 1);
        QVERIFY(registry.contains(QStringLiteral("system.resources")));
        const mirante::ItemType &type = registry.type(QStringLiteral("system.resources"));
        QCOMPARE(type.name(), QStringLiteral("System Resources"));
        QCOMPARE(type.rendering(), mirante::Rendering::Native);
        QVERIFY(!type.hasStatus());

        const QIcon icon = type.icon();
        QVERIFY(!icon.isNull());
        QVERIFY(!icon.pixmap(16, 16).isNull());
    }

    void fieldKeys()
    {
        mirante::testing::TestRegistrar registry;
        registry.registerModule(QStringLiteral("system"), &mirante::modules::system::registerModule);

        QStringList keys;
        for (const mirante::Field &field : registry.type(QStringLiteral("system.resources")).fields())
            keys << field.key;
        QCOMPARE(keys, (QStringList{QStringLiteral("showCpu"), QStringLiteral("showMemory"),
                                    QStringLiteral("showDisks"), QStringLiteral("disks"),
                                    QStringLiteral("interval")}));
    }
};

QTEST_MAIN(TestRegister)
#include "tst_register.moc"
