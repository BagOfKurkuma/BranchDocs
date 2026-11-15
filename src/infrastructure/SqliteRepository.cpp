#include "SqliteRepository.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <chrono>
#include <stdexcept>

using namespace branchdocs::domain;

namespace branchdocs::infrastructure {

SqliteRepository::SqliteRepository(std::string dbPath, std::string documentId)
    : m_documentId(std::move(documentId))
    , m_connectionName(QString("branchdocs_%1").arg(reinterpret_cast<quintptr>(this)))
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(QString::fromStdString(dbPath));
    if (!m_db.open())
        throw std::runtime_error("SqliteRepository: cannot open db");
    ensureSchema();
}

SqliteRepository::~SqliteRepository()
{
    if (m_db.isOpen())
        m_db.close();
    QSqlDatabase::removeDatabase(m_connectionName);
}

void SqliteRepository::ensureSchema()
{
    QSqlQuery q(m_db);
    q.exec("CREATE TABLE IF NOT EXISTS versions (version_id TEXT PRIMARY KEY, document_id TEXT NOT NULL, content TEXT NOT NULL, "
           "comment TEXT NOT NULL, timestamp INTEGER NOT NULL)");
    q.exec("CREATE INDEX IF NOT EXISTS idx_versions_doc ON versions(document_id)");
    q.exec("CREATE TABLE IF NOT EXISTS documents (document_id TEXT PRIMARY KEY, title TEXT NOT NULL)");
}

void SqliteRepository::save(const Version &version)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT OR REPLACE INTO versions (version_id, document_id, content, comment, timestamp) VALUES (?, ?, ?, ?, ?)");
    q.addBindValue(QString::fromStdString(version.id()));
    q.addBindValue(QString::fromStdString(m_documentId));
    q.addBindValue(QString::fromStdString(version.content()));
    q.addBindValue(QString::fromStdString(version.comment()));
    q.addBindValue(
        static_cast<qint64>(std::chrono::duration_cast<std::chrono::seconds>(version.timestamp().time_since_epoch()).count()));
    if (!q.exec())
        throw std::runtime_error("SqliteRepository::save: " + q.lastError().text().toStdString());
}

std::vector<Version> SqliteRepository::loadAll()
{
    std::vector<Version> result;
    QSqlQuery q(m_db);
    q.prepare("SELECT version_id, content, comment, timestamp FROM versions WHERE document_id = ? ORDER BY timestamp ASC");
    q.addBindValue(QString::fromStdString(m_documentId));
    if (!q.exec())
        return result;
    while (q.next()) {
        result.push_back(Version::fromPersisted(
            q.value(0).toString().toStdString(), q.value(1).toString().toStdString(), q.value(2).toString().toStdString(),
            std::chrono::system_clock::time_point(std::chrono::seconds(q.value(3).toLongLong()))));
    }
    return result;
}

bool SqliteRepository::remove(const std::string &versionId)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM versions WHERE version_id = ? AND document_id = ?");
    q.addBindValue(QString::fromStdString(versionId));
    q.addBindValue(QString::fromStdString(m_documentId));
    return q.exec() && q.numRowsAffected() > 0;
}

void SqliteRepository::saveDocumentTitle(const std::string &docId, const std::string &title)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT OR REPLACE INTO documents (document_id, title) VALUES (?, ?)");
    q.addBindValue(QString::fromStdString(docId));
    q.addBindValue(QString::fromStdString(title));
    q.exec();
}

std::string SqliteRepository::loadDocumentTitle(const std::string &docId)
{
    QSqlQuery q(m_db);
    q.prepare("SELECT title FROM documents WHERE document_id = ?");
    q.addBindValue(QString::fromStdString(docId));
    if (q.exec() && q.next())
        return q.value(0).toString().toStdString();
    return docId;
}

std::vector<std::pair<std::string, std::string>> SqliteRepository::getAllDocuments()
{
    std::vector<std::pair<std::string, std::string>> result;
    QSqlQuery q(m_db);
    if (q.exec("SELECT document_id, title FROM documents")) {
        while (q.next()) {
            result.push_back({q.value(0).toString().toStdString(), q.value(1).toString().toStdString()});
        }
    }
    return result;
}

} // namespace branchdocs::infrastructure