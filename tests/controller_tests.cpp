#include <QSignalSpy>
#include <QtTest>

#include "app/EditorController.h"

using namespace branchdocs;

class ControllerTests : public QObject
{
        Q_OBJECT
    private slots:
        void editEmitsModifiedTrue()
        {
            EditorController c(domain::Document("doc"));
            QSignalSpy spy(&c, &EditorController::modifiedStateChanged);
            c.handleEdit("hello");
            QCOMPARE(spy.count(), 1);
            QCOMPARE(spy.first().first().toBool(), true);
            QCOMPARE(QString::fromStdString(c.document().content()), QString("hello"));
        }

        void commitEmitsVersionAdded()
        {
            EditorController c(domain::Document("doc"));
            c.handleEdit("hello");
            QSignalSpy spy(&c, &EditorController::versionAdded);
            c.handleCommit("перша версія");
            QCOMPARE(spy.count(), 1);
            QCOMPARE(spy.first().at(2).toString(), QString("перша версія"));
        }

        void commitResetsModifiedState()
        {
            EditorController c(domain::Document("doc"));
            c.handleEdit("hello");
            QSignalSpy spy(&c, &EditorController::modifiedStateChanged);
            c.handleCommit("v1");
            QCOMPARE(spy.count(), 1);
            QCOMPARE(spy.first().first().toBool(), false);
        }

        void newDocumentEmitsThreeSignals()
        {
            EditorController c(domain::Document("old"));
            c.handleEdit("something");
            QSignalSpy contentSpy(&c, &EditorController::contentChanged);
            QSignalSpy historySpy(&c, &EditorController::historyCleared);
            QSignalSpy modSpy(&c, &EditorController::modifiedStateChanged);
            c.handleNewDocument("new title");
            QCOMPARE(contentSpy.count(), 1);
            QCOMPARE(contentSpy.first().first().toString(), QString(""));
            QCOMPARE(historySpy.count(), 1);
            QCOMPARE(modSpy.count(), 1);
            QCOMPARE(modSpy.first().first().toBool(), false);
            QCOMPARE(QString::fromStdString(c.document().title()), QString("new title"));
            QCOMPARE(c.document().history().size(), std::size_t(0));
        }

        void commitWithoutChangesDoesNothing()
        {
            EditorController c(domain::Document("doc"));
            QSignalSpy spy(&c, &EditorController::versionAdded);
            c.handleCommit("порожній коміт");
            QCOMPARE(spy.count(), 0);
            QCOMPARE(c.document().history().size(), std::size_t(0));
        }
};

QTEST_APPLESS_MAIN(ControllerTests)
#include "controller_tests.moc"