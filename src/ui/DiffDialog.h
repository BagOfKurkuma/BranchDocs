#pragma once
#include <QDialog>

class QTableWidget;

namespace branchdocs {
class DiffDialog : public QDialog
{
        Q_OBJECT
    public:
        DiffDialog(const QString &versionA, const QString &versionB, QWidget *parent = nullptr);

    private:
        QTableWidget *m_table = nullptr;
};
} // namespace branchdocs