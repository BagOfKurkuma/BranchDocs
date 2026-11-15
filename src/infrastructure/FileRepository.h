#pragma once
#include <filesystem>

#include "domain/IRepository.h"

namespace branchdocs::infrastructure {

class FileRepository : public domain::IRepository
{
    public:
        FileRepository(std::filesystem::path baseDir, std::string documentId);
        void save(const domain::Version &version) override;
        std::vector<domain::Version> loadAll() override;
        bool remove(const std::string &versionId) override;

        void saveDocumentTitle(const std::string &docId, const std::string &title) override;
        std::string loadDocumentTitle(const std::string &docId) override;
        std::vector<std::pair<std::string, std::string>> getAllDocuments() override;

    private:
        std::filesystem::path m_baseDir;
        std::filesystem::path m_versionsDir;
        std::string m_documentId;
};

} // namespace branchdocs::infrastructure