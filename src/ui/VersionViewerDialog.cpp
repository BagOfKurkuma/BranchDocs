#include "VersionViewerDialog.h"
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QVBoxLayout>

namespace branchdocs {
VersionViewerDialog::VersionViewerDialog(const QString &title, const QString &content, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(title);
    resize(800, 600);
    auto *layout = new QVBoxLayout(this);
    m_textEdit = new QPlainTextEdit(this);
    m_textEdit->setPlainText(content);
    m_textEdit->setReadOnly(true);
    layout->addWidget(m_textEdit);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
} // namespace branchdocs