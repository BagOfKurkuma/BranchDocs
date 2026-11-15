#include "MainWindow.h"
#include "app/EditorController.h"

#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

namespace branchdocs {

MainWindow::MainWindow(EditorController* controller, QWidget* parent)
    : QMainWindow(parent)
    , m_controller(controller)
{
    setWindowTitle("BranchDocs - контроль версій текстових документів");
    resize(1000, 640);

    auto* central = new QWidget(this);
    auto* root    = new QHBoxLayout(central);

    auto* left   = new QVBoxLayout();
    auto* topBar = new QHBoxLayout();

    m_newDocButton = new QPushButton("Новий документ", this);
    m_commitButton = new QPushButton("Commit", this);
    m_commitButton->setEnabled(false);
    topBar->addWidget(m_newDocButton);
    topBar->addWidget(m_commitButton);
    topBar->addStretch();

    m_textEdit = new QTextEdit(this);
    m_textEdit->setPlaceholderText("Введіть текст документа...");

    left->addLayout(topBar);
    left->addWidget(m_textEdit);

    auto* right = new QVBoxLayout();
    right->addWidget(new QLabel("Історія версій (поточний сеанс):", this));
    m_versionList = new QListWidget(this);
    right->addWidget(m_versionList);

    auto* leftWidget  = new QWidget(central);  leftWidget->setLayout(left);
    auto* rightWidget = new QWidget(central);  rightWidget->setLayout(right);

    auto* splitter = new QSplitter(Qt::Horizontal, central);
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    root->addWidget(splitter);

    setCentralWidget(central);

    m_statusLabel = new QLabel("Готово", this);
    statusBar()->addWidget(m_statusLabel);

    connect(m_textEdit,     &QTextEdit::textChanged, this, &MainWindow::onTextChanged);
    connect(m_commitButton, &QPushButton::clicked,   this, &MainWindow::onCommitClicked);
    connect(m_newDocButton, &QPushButton::clicked,   this, &MainWindow::onNewDocumentClicked);

    connect(m_controller, &EditorController::versionAdded,
            this, &MainWindow::onVersionAdded);
    connect(m_controller, &EditorController::historyCleared,
            this, &MainWindow::onHistoryCleared);
    connect(m_controller, &EditorController::modifiedStateChanged,
            this, &MainWindow::onModifiedStateChanged);
    connect(m_controller, &EditorController::contentChanged,
            this, [this](const QString& c) {
                m_suppressTextSignal = true;
                m_textEdit->setPlainText(c);
                m_suppressTextSignal = false;
            });

    onNewDocumentClicked();
}

void MainWindow::onTextChanged() {
    if (m_suppressTextSignal) return;
    m_controller->handleEdit(m_textEdit->toPlainText());
}

void MainWindow::onCommitClicked() {
    bool ok = false;
    const QString comment = QInputDialog::getText(
        this, "Коментар до версії", "Опишіть зміни:",
        QLineEdit::Normal, QString(), &ok);
    if (!ok) return;
    m_controller->handleCommit(comment);
}

void MainWindow::onNewDocumentClicked() {
    bool ok = false;
    const QString title = QInputDialog::getText(
        this, "Новий документ", "Заголовок документа:",
        QLineEdit::Normal, "Без назви", &ok);
    if (!ok) return;
    m_controller->handleNewDocument(title);
}

void MainWindow::onVersionAdded(const QString& id,
                                const QString& timestamp,
                                const QString& comment)
{
    const QString shortId = id.left(8);
    const QString text = QString("[%1]  %2  -  %3")
        .arg(shortId, timestamp,
             comment.isEmpty() ? QStringLiteral("(без коментаря)") : comment);
    m_versionList->addItem(text);
    m_statusLabel->setText(QString("Додано версію %1").arg(shortId));
}

void MainWindow::onHistoryCleared() {
    m_versionList->clear();
    m_statusLabel->setText("Створено новий документ");
}

void MainWindow::onModifiedStateChanged(bool modified) {
    m_commitButton->setEnabled(modified);
    const QString title = QString::fromStdString(m_controller->document().title());
    setWindowTitle(QString("BranchDocs - %1%2").arg(title, modified ? " *" : ""));
}

} // namespace branchdocs
