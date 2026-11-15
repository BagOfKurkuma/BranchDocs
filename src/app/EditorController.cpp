#include "EditorController.h"
#include <stdexcept>
#include <utility>

#include "RepositoryFactory.h"
#include "domain/commands/MergeCommitCommand.h"
#include "domain/commands/TextEditCommand.h"

namespace branchdocs {

EditorController::EditorController(std::shared_ptr<const RepositoryFactory> factory, StorageType initialType, domain::Document doc,
                                   QObject *parent)
    : QObject(parent)
    , m_factory(std::move(factory))
    , m_repository(m_factory ? m_factory->create(initialType, doc.id()) : nullptr)
    , m_document(std::move(doc))
    , m_storageType(initialType)
{
}

EditorController::EditorController(domain::Document doc, QObject *parent)
    : EditorController(nullptr, StorageType::File, std::move(doc), parent)
{
}

std::vector<std::pair<std::string, std::string>> EditorController::getAllDocuments() const
{
    return m_repository ? m_repository->getAllDocuments() : std::vector<std::pair<std::string, std::string>>{};
}

void EditorController::loadHistoryFromRepository()
{
    if (!m_repository)
        return;
    try {
        for (const auto &v : m_repository->loadAll()) {
            m_document.history().addVersion(v);
            emit versionAdded(QString::fromStdString(v.id()), QString::fromStdString(v.formattedTimestamp()),
                              QString::fromStdString(v.comment()));
        }
        emit historyReloaded();
    } catch (const std::exception &e) {
        emit errorOccurred(QString("Помилка завантаження: %1").arg(e.what()));
    }
}

void EditorController::handleEdit(const QString &newContent)
{
    m_document.edit(newContent.toStdString());
    emit modifiedStateChanged(m_document.isModified());
}

void EditorController::handleCommit(const QString &comment)
{
    if (!m_document.isModified())
        return;
    try {
        const auto v = m_document.createSnapshot(comment.toStdString());
        if (m_repository)
            m_repository->save(v);
        emit versionAdded(QString::fromStdString(v.id()), QString::fromStdString(v.formattedTimestamp()),
                          QString::fromStdString(v.comment()));
    } catch (const std::exception &e) {
        emit errorOccurred(QString("Commit failed: %1").arg(e.what()));
    }
    emit modifiedStateChanged(m_document.isModified());
}

void EditorController::handleNewDocument(const QString &title)
{
    m_document = domain::Document(title.toStdString());
    m_commandStack.clear();
    if (m_factory) {
        m_repository = m_factory->create(m_storageType, m_document.id());
        m_repository->saveDocumentTitle(m_document.id(), title.toStdString());
    }
    emit contentChanged(QString());
    emit historyCleared();
    emit modifiedStateChanged(false);
    notifyUndoRedo();
}

void EditorController::handleOpenDocument(const QString &id, const QString &title)
{
    m_document = domain::Document(id.toStdString(), title.toStdString());
    m_commandStack.clear();

    if (m_factory) {
        m_repository = m_factory->create(m_storageType, id.toStdString());
    }

    m_document.history().clear();
    loadHistoryFromRepository();

    auto latest = m_document.history().getLatestVersion();
    if (latest.has_value()) {
        m_document.restoreState(latest->content(), false);
        emit contentChanged(QString::fromStdString(m_document.content()));
    } else {
        emit contentChanged(QString());
    }

    emit modifiedStateChanged(false);
    notifyUndoRedo();
}

void EditorController::handleUndo()
{
    if (!m_commandStack.canUndo())
        return;
    m_commandStack.undo();
    emit contentChanged(QString::fromStdString(m_document.content()));
    emit modifiedStateChanged(m_document.isModified());
    notifyUndoRedo();
}

void EditorController::handleRedo()
{
    if (!m_commandStack.canRedo())
        return;
    m_commandStack.redo();
    emit contentChanged(QString::fromStdString(m_document.content()));
    emit modifiedStateChanged(m_document.isModified());
    notifyUndoRedo();
}

void EditorController::handleRestore(const QString &versionId)
{
    const domain::Version *v = m_document.history().findById(versionId.toStdString());
    if (!v)
        return;
    auto cmd = std::make_unique<domain::TextEditCommand>(m_document, v->content());
    m_commandStack.execute(std::move(cmd));
    emit contentChanged(QString::fromStdString(m_document.content()));
    emit modifiedStateChanged(m_document.isModified());
    notifyUndoRedo();
}

void EditorController::handleMergeCommit(const QString &mergedContent, const QString &comment)
{
    auto cmd = std::make_unique<domain::MergeCommitCommand>(m_document, mergedContent.toStdString(), comment.toStdString());
    m_commandStack.execute(std::move(cmd));
    const auto &versions = m_document.history().getAllVersions();
    if (!versions.empty()) {
        const auto &v = versions.back();
        if (m_repository) {
            try {
                m_repository->save(v);
            } catch (const std::exception &e) {
                emit errorOccurred(QString("Merge save error: %1").arg(e.what()));
            }
        }
        emit versionAdded(QString::fromStdString(v.id()), QString::fromStdString(v.formattedTimestamp()),
                          QString::fromStdString(v.comment()));
    }
    emit contentChanged(QString::fromStdString(m_document.content()));
    emit modifiedStateChanged(m_document.isModified());
    notifyUndoRedo();
}

void EditorController::handleSwitchStorage(StorageType type)
{
    if (!m_factory || type == m_storageType)
        return;
    try {
        auto newRepo = m_factory->create(type, m_document.id());
        for (const auto &v : m_document.history().getAllVersions()) {
            newRepo->save(v);
        }
        newRepo->saveDocumentTitle(m_document.id(), m_document.title());

        m_repository = std::move(newRepo);
        m_storageType = type;
        m_factory->writeConfig(type);
        m_document.history().clear();
        for (const auto &v : m_repository->loadAll()) {
            m_document.history().addVersion(v);
        }
        emit historyReloaded();
        for (const auto &v : m_document.history().getAllVersions()) {
            emit versionAdded(QString::fromStdString(v.id()), QString::fromStdString(v.formattedTimestamp()),
                              QString::fromStdString(v.comment()));
        }
        emit storageChanged(type);
    } catch (const std::exception &e) {
        emit errorOccurred(QString("Storage switch error: %1").arg(e.what()));
    }
}

void EditorController::handleDeleteVersion(const QString &versionId)
{
    if (m_repository) {
        m_repository->remove(versionId.toStdString());
    }
    m_document.history().removeVersion(versionId.toStdString());
    emit historyReloaded();
    for (const auto &v : m_document.history().getAllVersions()) {
        emit versionAdded(QString::fromStdString(v.id()), QString::fromStdString(v.formattedTimestamp()),
                          QString::fromStdString(v.comment()));
    }
}

void EditorController::handleExportVersion(const QString &versionId, const QString &filePath)
{
    const domain::Version *v = m_document.history().findById(versionId.toStdString());
    if (!v)
        return;

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(v->content().c_str());
        file.close();
    } else {
        emit errorOccurred("Не вдалося зберегти файл.");
    }
}

void EditorController::notifyUndoRedo()
{
    emit undoAvailableChanged(m_commandStack.canUndo());
    emit redoAvailableChanged(m_commandStack.canRedo());
}

} // namespace branchdocs