#include <QRegularExpression>
#include <QtTest>
#include <stdexcept>

#include "domain/Document.h"
#include "domain/Version.h"
#include "domain/VersionHistory.h"

using namespace branchdocs::domain;

class DomainTests : public QObject
{
        Q_OBJECT
    private slots:
        void versionIsImmutable()
        {
            Version v("hello", "first commit");
            QVERIFY(!v.id().empty());
            QCOMPARE(QString::fromStdString(v.content()), QString("hello"));
            QCOMPARE(QString::fromStdString(v.comment()), QString("first commit"));
        }

        void versionIdIsUuidLike()
        {
            Version v("x", "y");
            QCOMPARE(v.id().size(), std::size_t(36));
            QCOMPARE(v.id()[8], '-');
            QCOMPARE(v.id()[13], '-');
            QCOMPARE(v.id()[14], '4');
            QCOMPARE(v.id()[18], '-');
            QCOMPARE(v.id()[23], '-');
            const char variant = v.id()[19];
            QVERIFY(variant == '8' || variant == '9' || variant == 'a' || variant == 'b');
        }

        void twoVersionsHaveDifferentIds()
        {
            Version a("x", "1"), b("x", "2");
            QVERIFY(a.id() != b.id());
        }

        void versionPreservesUnicodeAndNewlines()
        {
            const std::string content = "Привіт\nсвіт\t\u00e9\u4e2d";
            Version v(content, "тест");
            QCOMPARE(v.content(), content);
        }

        void versionTimestampIsSet()
        {
            const auto before = std::chrono::system_clock::now();
            Version v("x", "y");
            const auto after = std::chrono::system_clock::now();
            QVERIFY(v.timestamp() >= before);
            QVERIFY(v.timestamp() <= after);
        }

        void versionFormattedTimestampMatchesPattern()
        {
            Version v("x", "y");
            const QString ts = QString::fromStdString(v.formattedTimestamp());
            QVERIFY(QRegularExpression("^\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}$").match(ts).hasMatch());
        }

        void newHistoryIsEmpty()
        {
            VersionHistory h;
            QCOMPARE(h.size(), std::size_t(0));
            QVERIFY(h.getAllVersions().empty());
            QVERIFY(!h.getLatestVersion().has_value());
        }

        void historyKeepsInsertionOrder()
        {
            Document doc("Test");
            doc.edit("a");
            doc.createSnapshot("1");
            doc.edit("b");
            doc.createSnapshot("2");
            doc.edit("c");
            doc.createSnapshot("3");

            const auto &all = doc.history().getAllVersions();
            QCOMPARE(all.size(), std::size_t(3));
            QCOMPARE(QString::fromStdString(all[0].comment()), QString("1"));
            QCOMPARE(QString::fromStdString(all[1].comment()), QString("2"));
            QCOMPARE(QString::fromStdString(all[2].comment()), QString("3"));
        }

        void documentConstructorSetsFields()
        {
            Document doc("Мій документ");
            QVERIFY(!doc.id().empty());
            QCOMPARE(QString::fromStdString(doc.title()), QString("Мій документ"));
            QVERIFY(doc.content().empty());
            QVERIFY(!doc.isModified());
        }

        void twoDocumentsHaveDifferentIds()
        {
            Document a("A"), b("B");
            QVERIFY(a.id() != b.id());
        }

        void documentEditMarksModified()
        {
            Document doc("Test");
            QVERIFY(!doc.isModified());
            doc.edit("some text");
            QVERIFY(doc.isModified());
            QCOMPARE(QString::fromStdString(doc.content()), QString("some text"));
        }

        void editWithSameContentIsNoOp()
        {
            Document doc("Test");
            doc.edit("same");
            doc.createSnapshot("v1");
            doc.edit("same");
            QVERIFY(!doc.isModified());
        }

        void editWithEmptyStringIsAllowed()
        {
            Document doc("Test");
            doc.edit("something");
            doc.createSnapshot("v1");
            doc.edit("");
            QVERIFY(doc.isModified());
            QCOMPARE(doc.content().size(), std::size_t(0));
        }

        void snapshotAddsToHistoryAndResetsModified()
        {
            Document doc("Test");
            doc.edit("v1");
            doc.createSnapshot("first");
            QCOMPARE(doc.history().size(), std::size_t(1));
            QVERIFY(!doc.isModified());

            doc.edit("v2");
            doc.createSnapshot("second");
            QCOMPARE(doc.history().size(), std::size_t(2));
        }

        void snapshotContentEqualsCurrentDocument()
        {
            Document doc("Test");
            doc.edit("hello world");
            auto v = doc.createSnapshot("v1");
            QCOMPARE(QString::fromStdString(v.content()), QString("hello world"));
        }

        void snapshotCapturesStateAtMomentOfCommit()
        {
            Document doc("Test");
            doc.edit("version 1");
            doc.createSnapshot("v1");
            doc.edit("version 2 totally different text");
            QCOMPARE(QString::fromStdString(doc.history().getAllVersions()[0].content()), QString("version 1"));
        }

        void multipleEditsOneSnapshotCapturesFinalState()
        {
            Document doc("Test");
            doc.edit("a");
            doc.edit("ab");
            doc.edit("abc");
            doc.createSnapshot("v1");
            QCOMPARE(QString::fromStdString(doc.history().getAllVersions()[0].content()), QString("abc"));
        }

        void snapshotWithoutChangesThrows()
        {
            Document doc("Test");
            doc.edit("x");
            doc.createSnapshot("v1");
            QVERIFY_EXCEPTION_THROWN(doc.createSnapshot("v2"), std::logic_error);
            QCOMPARE(doc.history().size(), std::size_t(1));
        }
};

QTEST_APPLESS_MAIN(DomainTests)
#include "domain_tests.moc"