#include "EditorController.h"
#include <utility>

namespace branchdocs {

EditorController::EditorController(domain::Document doc, QObject *parent) : QObject(parent), m_document(std::move(doc)) {}

void EditorController::handleEdit(const QString &newContent)
{
    m_document.edit(newContent.toStdString());
    emit modifiedStateChanged(m_document.isModified());
}

void EditorController::handleCommit(const QString &comment)
{
    if (!m_document.isModified()) {
        return;
    }
    const auto v = m_document.createSnapshot(comment.toStdString());
    emit versionAdded(QString::fromStdString(v.id()), QString::fromStdString(v.formattedTimestamp()),
                      QString::fromStdString(v.comment()));
    emit modifiedStateChanged(m_document.isModified());
}

void EditorController::handleNewDocument(const QString &title)
{
    m_document = domain::Document(title.toStdString());
    emit contentChanged(QString());
    emit historyCleared();
    emit modifiedStateChanged(false);
}

} // namespace branchdocs
