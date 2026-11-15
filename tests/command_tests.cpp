#include <QtTest>

#include "domain/Document.h"
#include "domain/commands/CommandStack.h"
#include "domain/commands/MergeCommitCommand.h"
#include "domain/commands/TextEditCommand.h"
using namespace branchdocs::domain;

class CommandTests : public QObject
{
        Q_OBJECT
    private slots:
        void textEditCommandRoundTrip()
        {
            Document doc("doc");
            doc.edit("initial");
            CommandStack stack;

            stack.execute(std::make_unique<TextEditCommand>(doc, "modified"));
            QCOMPARE(QString::fromStdString(doc.content()), QString("modified"));

            stack.undo();
            QCOMPARE(QString::fromStdString(doc.content()), QString("initial"));

            stack.redo();
            QCOMPARE(QString::fromStdString(doc.content()), QString("modified"));
        }

        void newCommandClearsRedo()
        {
            Document doc("doc");
            doc.edit("a");
            CommandStack stack;
            stack.execute(std::make_unique<TextEditCommand>(doc, "b"));
            stack.undo();
            QVERIFY(stack.canRedo());
            stack.execute(std::make_unique<TextEditCommand>(doc, "c"));
            QVERIFY(!stack.canRedo());
        }

        void mergeCommitCommandRollsBackVersion()
        {
            Document doc("doc");
            doc.edit("v1");
            doc.createSnapshot("base");
            doc.edit("v2");
            CommandStack stack;

            stack.execute(std::make_unique<MergeCommitCommand>(doc, "merged", "merge comment"));
            QCOMPARE(doc.history().size(), std::size_t(2));
            QCOMPARE(QString::fromStdString(doc.history().getAllVersions().back().comment()), QString("merge comment"));

            stack.undo();
            QCOMPARE(doc.history().size(), std::size_t(1));
            QCOMPARE(QString::fromStdString(doc.content()), QString("v2"));
        }

        void mergeCommitRedoRestoresVersion()
        {
            Document doc("doc");
            doc.edit("v1");
            doc.createSnapshot("base");
            doc.edit("v2");
            CommandStack stack;
            stack.execute(std::make_unique<MergeCommitCommand>(doc, "merged", "merge"));
            stack.undo();
            stack.redo();
            QCOMPARE(doc.history().size(), std::size_t(2));
            QCOMPARE(QString::fromStdString(doc.content()), QString("merged"));
        }
};
QTEST_APPLESS_MAIN(CommandTests)
#include "command_tests.moc"