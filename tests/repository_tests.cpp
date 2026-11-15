#include <QTemporaryDir>
#include <QtTest>

#include "domain/IRepository.h"
#include "infrastructure/FileRepository.h"

using namespace branchdocs;
using namespace branchdocs::domain;

class RepositoryTests : public QObject
{
        Q_OBJECT
    private slots:
        void roundTripSaveLoad()
        {
            QTemporaryDir tmp;
            infrastructure::FileRepository repo(tmp.path().toStdString(), "doc-1");

            Version v("hello world", "first commit");
            repo.save(v);

            auto loaded = repo.loadAll();
            QCOMPARE(loaded.size(), std::size_t(1));
            QCOMPARE(QString::fromStdString(loaded[0].id()), QString::fromStdString(v.id()));
            QCOMPARE(QString::fromStdString(loaded[0].content()), QString("hello world"));
            QCOMPARE(QString::fromStdString(loaded[0].comment()), QString("first commit"));
        }

        void loadAllReturnsChronologicalOrder()
        {
            QTemporaryDir tmp;
            infrastructure::FileRepository repo(tmp.path().toStdString(), "doc-2");

            Version v1("a", "1");
            Version v2("b", "2");
            Version v3("c", "3");

            repo.save(v3);
            repo.save(v1);
            repo.save(v2);

            auto loaded = repo.loadAll();
            QCOMPARE(loaded.size(), std::size_t(3));
            QVERIFY(loaded[0].timestamp() <= loaded[1].timestamp());
            QVERIFY(loaded[1].timestamp() <= loaded[2].timestamp());
        }

        void removeDeletesPersistedVersion()
        {
            QTemporaryDir tmp;
            infrastructure::FileRepository repo(tmp.path().toStdString(), "doc-3");

            Version v("x", "y");
            repo.save(v);
            QCOMPARE(repo.loadAll().size(), std::size_t(1));

            QVERIFY(repo.remove(v.id()));
            QVERIFY(repo.loadAll().empty());
        }
};

QTEST_APPLESS_MAIN(RepositoryTests)
#include "repository_tests.moc"