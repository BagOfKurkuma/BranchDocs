#pragma once
#include <string>
#include <utility>
#include <vector>

#include "Version.h"

namespace branchdocs::domain {

class IRepository
{
    public:
        virtual ~IRepository() = default;
        virtual void save(const Version &version) = 0;
        virtual std::vector<Version> loadAll() = 0;
        virtual bool remove(const std::string &versionId) = 0;

        virtual void saveDocumentTitle(const std::string &docId, const std::string &title) = 0;
        virtual std::string loadDocumentTitle(const std::string &docId) = 0;
        virtual std::vector<std::pair<std::string, std::string>> getAllDocuments() = 0;
};

} // namespace branchdocs::domain