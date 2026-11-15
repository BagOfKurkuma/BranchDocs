#pragma once
#include <QDialog>

class QPlainTextEdit;

namespace branchdocs {
class VersionViewerDialog : public QDialog
{
        Q_OBJECT
    public:
        VersionViewerDialog(const QString &title, const QString &content, QWidget *parent = nullptr);

    private:
        QPlainTextEdit *m_textEdit = nullptr;
};
} // namespace branchdocs