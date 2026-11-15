#include <QtTest>

#include "domain/Document.h"

using namespace branchdocs::domain;

class IntegrationTests : public QObject
{
        Q_OBJECT
    private slots:
        void typicalUserSession()
        {
            Document doc("Курсова робота");

            doc.edit("Вступ\nРозділ 1");
            doc.createSnapshot("чорновий варіант");

            doc.edit("Вступ\nРозділ 1\nРозділ 2");
            doc.createSnapshot("додано розділ 2");

            doc.edit("Вступ\nРозділ 1\nРозділ 2 (виправлено)");
            doc.createSnapshot("виправлення");

            QCOMPARE(doc.history().size(), std::size_t(3));
            QCOMPARE(QString::fromStdString(doc.history().getAllVersions()[0].content()), QString("Вступ\nРозділ 1"));
            QCOMPARE(QString::fromStdString(doc.history().getLatestVersion()->content()), QString::fromStdString(doc.content()));
        }

        void editWithoutCommitLeavesNoTrace()
        {
            Document doc("Doc");
            doc.edit("first");
            doc.createSnapshot("v1");
            doc.edit("second");
            QCOMPARE(doc.history().size(), std::size_t(1));
            QCOMPARE(QString::fromStdString(doc.content()), QString("second"));
        }

        void reopeningDocumentDoesNotAffectHistory()
        {
            Document doc("Doc");
            doc.edit("hello");
            doc.createSnapshot("v1");
            QCOMPARE(doc.history().size(), std::size_t(1));
            QCOMPARE(QString::fromStdString(doc.content()), QString("hello"));
            QVERIFY(!doc.isModified());
        }
};

QTEST_APPLESS_MAIN(IntegrationTests)
#include "integration_tests.moc"