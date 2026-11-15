#pragma once
#include <QMainWindow>

class QTextEdit;
class QListWidget;
class QPushButton;
class QLabel;

namespace branchdocs {

class EditorController;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(EditorController* controller, QWidget* parent = nullptr);

private slots:
    void onTextChanged();
    void onCommitClicked();
    void onNewDocumentClicked();
    void onVersionAdded(const QString& id,
                        const QString& timestamp,
                        const QString& comment);
    void onHistoryCleared();
    void onModifiedStateChanged(bool modified);

private:
    EditorController* m_controller;
    QTextEdit*   m_textEdit      = nullptr;
    QListWidget* m_versionList   = nullptr;
    QPushButton* m_commitButton  = nullptr;
    QPushButton* m_newDocButton  = nullptr;
    QLabel*      m_statusLabel   = nullptr;
    bool         m_suppressTextSignal = false;
};

} // namespace branchdocs
