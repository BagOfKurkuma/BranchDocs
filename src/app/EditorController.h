#pragma once
#include <QObject>
#include <QString>

#include "domain/Document.h"

namespace branchdocs {

class EditorController : public QObject
{
        Q_OBJECT
    public:
        explicit EditorController(domain::Document doc, QObject *parent = nullptr);

        const domain::Document &document() const noexcept { return m_document; }
        domain::Document &document() noexcept { return m_document; }

    public slots:
        void handleEdit(const QString &newContent);
        void handleCommit(const QString &comment);
        void handleNewDocument(const QString &title);

    signals:
        void contentChanged(const QString &content);
        void versionAdded(const QString &id, const QString &timestamp, const QString &comment);
        void historyCleared();
        void modifiedStateChanged(bool isModified);

    private:
        domain::Document m_document;
};

} // namespace branchdocs