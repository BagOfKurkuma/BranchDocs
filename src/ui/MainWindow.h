#pragma once
#include <QMainWindow>
#include <QPoint>
#include <QString>

#include "domain/StorageType.h"

class QTextEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLabel;
class QComboBox;
class QLineEdit;
class QTimer;
class QCloseEvent;

namespace branchdocs {

class EditorController;

class MainWindow : public QMainWindow
{
        Q_OBJECT
    public:
        explicit MainWindow(EditorController *controller, QWidget *parent = nullptr);

    protected:
        void closeEvent(QCloseEvent *event) override;

    private slots:
        void onTextChanged();
        void onCommitClicked();
        void onNewDocumentClicked();
        void onOpenDocumentClicked();
        void onUndoClicked();
        void onRedoClicked();
        void onMergeClicked();
        void onDiffClicked();
        void onStatsClicked();
        void onVersionDoubleClicked(QListWidgetItem *item);
        void onVersionContextMenu(const QPoint &pos);
        void onVersionAdded(const QString &id, const QString &timestamp, const QString &comment);
        void onHistoryCleared();
        void onModifiedStateChanged(bool modified);
        void onUndoAvailabilityChanged(bool available);
        void onRedoAvailabilityChanged(bool available);
        void onErrorOccurred(const QString &message);
        void onHistoryReloaded();
        void onStorageChanged(StorageType type);
        void filterHistory(const QString &text);
        void saveDraft();

    private:
        EditorController *m_controller;
        QTextEdit *m_textEdit = nullptr;
        QListWidget *m_versionList = nullptr;
        QPushButton *m_commitButton = nullptr;
        QPushButton *m_newDocButton = nullptr;
        QPushButton *m_openDocButton = nullptr;
        QPushButton *m_undoButton = nullptr;
        QPushButton *m_redoButton = nullptr;
        QPushButton *m_mergeButton = nullptr;
        QPushButton *m_diffButton = nullptr;
        QPushButton *m_statsButton = nullptr;
        QLabel *m_statusLabel = nullptr;
        QComboBox *m_storageCombo = nullptr;
        QLineEdit *m_searchEdit = nullptr;
        QTimer *m_autosaveTimer = nullptr;
        bool m_suppressTextSignal = false;
};

} // namespace branchdocs