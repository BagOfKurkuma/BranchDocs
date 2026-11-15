#include "FileRepository.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <algorithm>
#include <chrono>
#include <stdexcept>

namespace fs = std::filesystem;
using namespace branchdocs::domain;

namespace branchdocs::infrastructure {

FileRepository::FileRepository(fs::path baseDir, std::string documentId)
    : m_baseDir(baseDir)
    , m_versionsDir(baseDir / documentId / "versions")
    , m_documentId(std::move(documentId))
{
    std::error_code ec;
    fs::create_directories(m_versionsDir, ec);
}

void FileRepository::save(const Version &version)
{
    const auto path = m_versionsDir / (version.id() + ".json");
    QJsonObject obj;
    obj["schema_version"] = 1;
    obj["documentId"] = QString::fromStdString(m_documentId);
    obj["versionId"] = QString::fromStdString(version.id());
    obj["content"] = QString::fromStdString(version.content());
    obj["comment"] = QString::fromStdString(version.comment());
    obj["timestamp"] =
        static_cast<qint64>(std::chrono::duration_cast<std::chrono::seconds>(version.timestamp().time_since_epoch()).count());

    QSaveFile file(QString::fromStdString(path.string()));
    if (!file.open(QIODevice::WriteOnly))
        throw std::runtime_error("FileRepository::save: cannot open");
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    if (!file.commit())
        throw std::runtime_error("FileRepository::save: commit failed");
}

std::vector<Version> FileRepository::loadAll()
{
    std::vector<Version> unsorted;
    for (const auto &entry : fs::directory_iterator(m_versionsDir)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".json")
            continue;
        QFile f(QString::fromStdString(entry.path().string()));
        if (!f.open(QIODevice::ReadOnly))
            continue;
        const auto jsonDoc = QJsonDocument::fromJson(f.readAll());
        if (!jsonDoc.isObject())
            continue;
        const auto obj = jsonDoc.object();
        unsorted.push_back(Version::fromPersisted(
            obj["versionId"].toString().toStdString(), obj["content"].toString().toStdString(),
            obj["comment"].toString().toStdString(),
            std::chrono::system_clock::time_point(std::chrono::seconds(obj["timestamp"].toVariant().toLongLong()))));
    }
    std::vector<const Version *> sortedPtrs;
    sortedPtrs.reserve(unsorted.size());
    for (const auto &v : unsorted)
        sortedPtrs.push_back(&v);
    std::sort(sortedPtrs.begin(), sortedPtrs.end(),
              [](const Version *a, const Version *b) { return a->timestamp() < b->timestamp(); });
    std::vector<Version> result;
    result.reserve(sortedPtrs.size());
    for (const Version *p : sortedPtrs)
        result.push_back(*p);
    return result;
}

bool FileRepository::remove(const std::string &versionId)
{
    std::error_code ec;
    return fs::remove(m_versionsDir / (versionId + ".json"), ec);
}

void FileRepository::saveDocumentTitle(const std::string &docId, const std::string &title)
{
    auto dir = m_baseDir / docId;
    std::error_code ec;
    fs::create_directories(dir, ec);
    auto path = dir / "meta.json";

    QJsonObject obj;
    obj["title"] = QString::fromStdString(title);

    QSaveFile file(QString::fromStdString(path.string()));
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(obj).toJson());
        file.commit();
    }
}

std::string FileRepository::loadDocumentTitle(const std::string &docId)
{
    auto path = m_baseDir / docId / "meta.json";
    if (fs::exists(path)) {
        QFile f(QString::fromStdString(path.string()));
        if (f.open(QIODevice::ReadOnly)) {
            auto doc = QJsonDocument::fromJson(f.readAll());
            if (doc.isObject())
                return doc.object()["title"].toString().toStdString();
        }
    }
    return docId;
}

std::vector<std::pair<std::string, std::string>> FileRepository::getAllDocuments()
{
    std::vector<std::pair<std::string, std::string>> result;
    std::error_code ec;
    if (!fs::exists(m_baseDir))
        return result;

    for (const auto &entry : fs::directory_iterator(m_baseDir, ec)) {
        if (entry.is_directory()) {
            std::string id = entry.path().filename().string();
            std::string title = loadDocumentTitle(id);
            result.push_back({id, title});
        }
    }
    return result;
}

} // namespace branchdocs::infrastructure