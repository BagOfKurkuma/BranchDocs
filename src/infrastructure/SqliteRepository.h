#pragma once
#include <QSqlDatabase>

#include "domain/IRepository.h"

namespace branchdocs::infrastructure {

class SqliteRepository : public domain::IRepository
{
    public:
        SqliteRepository(std::string dbPath, std::string documentId);
        ~SqliteRepository() override;
        void save(const domain::Version &version) override;
        std::vector<domain::Version> loadAll() override;
        bool remove(const std::string &versionId) override;

        void saveDocumentTitle(const std::string &docId, const std::string &title) override;
        std::string loadDocumentTitle(const std::string &docId) override;
        std::vector<std::pair<std::string, std::string>> getAllDocuments() override;

    private:
        void ensureSchema();
        QSqlDatabase m_db;
        std::string m_documentId;
        QString m_connectionName;
};

} // namespace branchdocs::infrastructure