#pragma once
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <filesystem>
#include <memory>

#include "domain/IRepository.h"
#include "domain/StorageType.h"
#include "infrastructure/FileRepository.h"
#include "infrastructure/SqliteRepository.h"

namespace branchdocs {

class RepositoryFactory
{
    public:
        explicit RepositoryFactory(std::filesystem::path dataDir) : m_dataDir(std::move(dataDir)) {}

        std::unique_ptr<domain::IRepository> create(StorageType type, const std::string &documentId) const
        {
            switch (type) {
            case StorageType::Sqlite:
                return std::make_unique<infrastructure::SqliteRepository>((m_dataDir / "branchdocs.db").string(), documentId);
            case StorageType::File:
            default:
                return std::make_unique<infrastructure::FileRepository>(m_dataDir, documentId);
            }
        }

        StorageType readConfig() const
        {
            QFile f(QString::fromStdString((m_dataDir / "branchdocs_config.json").string()));
            if (!f.open(QIODevice::ReadOnly))
                return StorageType::File;
            const auto doc = QJsonDocument::fromJson(f.readAll());
            if (!doc.isObject())
                return StorageType::File;
            const QString raw = doc.object().value("storage").toString().toLower();
            return (raw == QLatin1String("sqlite")) ? StorageType::Sqlite : StorageType::File;
        }

        void writeConfig(StorageType type) const
        {
            QJsonObject obj;
            obj["storage"] = (type == StorageType::Sqlite) ? "sqlite" : "file";
            QFile f(QString::fromStdString((m_dataDir / "branchdocs_config.json").string()));
            if (f.open(QIODevice::WriteOnly))
                f.write(QJsonDocument(obj).toJson());
        }

    private:
        std::filesystem::path m_dataDir;
};

} // namespace branchdocs