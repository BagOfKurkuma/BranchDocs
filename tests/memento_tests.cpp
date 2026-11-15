#include <QtTest>

#include "domain/Document.h"
using namespace branchdocs::domain;

class MementoTests : public QObject
{
        Q_OBJECT
    private slots:
        void versionIsImmutable()
        {
            Version v("hello", "first");
            QCOMPARE(QString::fromStdString(v.content()), QString("hello"));
        }
        void restoreFromVersionRevertsContent()
        {
            Document doc("Test");
            doc.edit("v1");
            const auto v1 = doc.createSnapshot("first");
            doc.edit("v2");
            QCOMPARE(QString::fromStdString(doc.content()), QString("v2"));
            doc.restoreFrom(v1);
            QCOMPARE(QString::fromStdString(doc.content()), QString("v1"));
            QVERIFY(doc.isModified());
        }
        void restoreStateSetsExactFlag()
        {
            Document doc("Test");
            doc.edit("v1");
            doc.createSnapshot("first");
            doc.edit("v2");
            const auto preContent = doc.content();
            const bool preModified = doc.isModified();
            doc.restoreState("dummy", true);
            doc.restoreState(preContent, preModified);
            QCOMPARE(QString::fromStdString(doc.content()), QString("v2"));
            QVERIFY(doc.isModified());
        }
        void removeVersionById()
        {
            Document doc("Test");
            doc.edit("a");
            const auto v1 = doc.createSnapshot("1");
            doc.edit("b");
            doc.createSnapshot("2");
            QVERIFY(doc.history().removeVersion(v1.id()));
            QCOMPARE(doc.history().size(), std::size_t(1));
            QVERIFY(!doc.history().removeVersion("nonexistent"));
        }
};
QTEST_APPLESS_MAIN(MementoTests)
#include "memento_tests.moc"