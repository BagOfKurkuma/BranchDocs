#pragma once
#include <QDialog>
#include <QString>
#include <vector>

class QComboBox;
class QTableWidget;
class QPlainTextEdit;

namespace branchdocs {
struct MergeVersionItem
{
        QString id;
        QString label;
        QString content;
};

class MergeDialog : public QDialog
{
        Q_OBJECT
    public:
        MergeDialog(const std::vector<MergeVersionItem> &versions, QWidget *parent = nullptr);
        QString mergedText() const;

    private slots:
        void onVersionsChanged();
        void updatePreview();

    private:
        QString assembleResult() const;
        std::vector<MergeVersionItem> m_versions;
        QComboBox *m_comboA = nullptr;
        QComboBox *m_comboB = nullptr;
        QTableWidget *m_table = nullptr;
        QPlainTextEdit *m_preview = nullptr;
};
} // namespace branchdocs