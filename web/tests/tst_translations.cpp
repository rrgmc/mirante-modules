// The module carries its own translations, under the name that the app loads.
#include <QCoreApplication>
#include <QTest>
#include <QTranslator>

class TestTranslations : public QObject
{
    Q_OBJECT

private slots:
    void loadsItsPortuguese()
    {
        QTranslator translator;
        QVERIFY(translator.load(QStringLiteral(":/i18n/web_pt_BR")));
        QCOMPARE(translator.translate("web", "Web page"), QStringLiteral("Página web"));
    }
};

QTEST_APPLESS_MAIN(TestTranslations)
#include "tst_translations.moc"
