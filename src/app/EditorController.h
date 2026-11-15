#pragma once
#include <QObject>
#include <QString>
#include <memory>
#include <utility>
#include <vector>

#include "domain/Document.h"
#include "domain/IRepository.h"
#include "domain/StorageType.h"
#include "domain/commands/CommandStack.h"

namespace branchdocs {

class RepositoryFactory;

class EditorController : public QObject
{
        Q_OBJECT
    public:
        EditorController(std::shared_ptr<const RepositoryFactory> factory, StorageType initialType, domain::Document doc,
                         QObject *parent = nullptr);
        explicit EditorController(domain::Document doc, QObject *parent = nullptr);
        const domain::Document &document() const noexcept { return m_document; }
        domain::Document &document() noexcept { return m_document; }
        StorageType currentStorage() const noexcept { return m_storageType; }
        void loadHistoryFromRepository();
        std::vector<std::pair<std::string, std::string>> getAllDocuments() const;

    public slots:
        void handleEdit(const QString &newContent);
        void handleCommit(const QString &comment);
        void handleNewDocument(const QString &title);
        void handleOpenDocument(const QString &id, const QString &title);
        void handleUndo();
        void handleRedo();
        void handleRestore(const QString &versionId);
        void handleMergeCommit(const QString &mergedContent, const QString &comment);
        void handleSwitchStorage(StorageType type);
        void handleDeleteVersion(const QString &versionId);
        void handleExportVersion(const QString &versionId, const QString &filePath);

    signals:
        void contentChanged(const QString &content);
        void versionAdded(const QString &id, const QString &timestamp, const QString &comment);
        void historyCleared();
        void historyReloaded();
        void modifiedStateChanged(bool isModified);
        void undoAvailableChanged(bool available);
        void redoAvailableChanged(bool available);
        void errorOccurred(const QString &message);
        void storageChanged(StorageType type);

    private:
        void notifyUndoRedo();
        std::shared_ptr<const RepositoryFactory> m_factory;
        std::unique_ptr<domain::IRepository> m_repository;
        domain::Document m_document;
        domain::CommandStack m_commandStack;
        StorageType m_storageType = StorageType::File;
};

} // namespace branchdocs