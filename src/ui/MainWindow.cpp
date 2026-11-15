#include "MainWindow.h"
#include <QCloseEvent>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTextEdit>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <filesystem>

#include "app/EditorController.h"
#include "ui/DiffDialog.h"
#include "ui/MergeDialog.h"
#include "ui/VersionViewerDialog.h"

namespace branchdocs {

MainWindow::MainWindow(EditorController *controller, QWidget *parent) : QMainWindow(parent), m_controller(controller)
{
    setWindowTitle("BranchDocs");
    resize(1100, 700);

    auto *central = new QWidget(this);
    auto *root = new QHBoxLayout(central);

    auto *left = new QVBoxLayout();
    auto *topBar = new QHBoxLayout();

    m_newDocButton = new QPushButton("Новий", this);
    m_openDocButton = new QPushButton("Відкрити", this);
    m_commitButton = new QPushButton("Commit", this);
    m_commitButton->setEnabled(false);
    topBar->addWidget(m_newDocButton);
    topBar->addWidget(m_openDocButton);
    topBar->addWidget(m_commitButton);
    topBar->addSpacing(10);

    m_undoButton = new QPushButton("Undo", this);
    m_redoButton = new QPushButton("Redo", this);
    m_undoButton->setEnabled(false);
    m_redoButton->setEnabled(false);
    topBar->addWidget(m_undoButton);
    topBar->addWidget(m_redoButton);
    topBar->addSpacing(10);

    m_diffButton = new QPushButton("Порівняти", this);
    m_mergeButton = new QPushButton("Merge…", this);
    m_statsButton = new QPushButton("Статистика", this);
    topBar->addWidget(m_diffButton);
    topBar->addWidget(m_mergeButton);
    topBar->addWidget(m_statsButton);
    topBar->addStretch();

    topBar->addWidget(new QLabel("Сховище:", this));
    m_storageCombo = new QComboBox(this);
    m_storageCombo->addItem("JSON-файли", static_cast<int>(StorageType::File));
    m_storageCombo->addItem("SQLite", static_cast<int>(StorageType::Sqlite));
    m_storageCombo->setCurrentIndex(m_controller->currentStorage() == StorageType::File ? 0 : 1);
    topBar->addWidget(m_storageCombo);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setPlaceholderText("Введіть текст документа…");
    m_textEdit->setAcceptRichText(false);
    left->addLayout(topBar);
    left->addWidget(m_textEdit);

    auto *right = new QVBoxLayout();
    right->addWidget(new QLabel("Пошук по історії:", this));
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Введіть коментар або дату...");
    right->addWidget(m_searchEdit);

    right->addWidget(new QLabel("Історія версій:", this));
    m_versionList = new QListWidget(this);
    m_versionList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_versionList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    right->addWidget(m_versionList);

    auto *leftWidget = new QWidget(central);
    leftWidget->setLayout(left);
    auto *rightWidget = new QWidget(central);
    rightWidget->setLayout(right);

    auto *splitter = new QSplitter(Qt::Horizontal, central);
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    root->addWidget(splitter);
    setCentralWidget(central);

    m_statusLabel = new QLabel("Готово", this);
    statusBar()->addWidget(m_statusLabel);

    connect(m_textEdit, &QTextEdit::textChanged, this, &MainWindow::onTextChanged);
    connect(m_commitButton, &QPushButton::clicked, this, &MainWindow::onCommitClicked);
    connect(m_newDocButton, &QPushButton::clicked, this, &MainWindow::onNewDocumentClicked);
    connect(m_openDocButton, &QPushButton::clicked, this, &MainWindow::onOpenDocumentClicked);
    connect(m_undoButton, &QPushButton::clicked, this, &MainWindow::onUndoClicked);
    connect(m_redoButton, &QPushButton::clicked, this, &MainWindow::onRedoClicked);
    connect(m_mergeButton, &QPushButton::clicked, this, &MainWindow::onMergeClicked);
    connect(m_diffButton, &QPushButton::clicked, this, &MainWindow::onDiffClicked);
    connect(m_statsButton, &QPushButton::clicked, this, &MainWindow::onStatsClicked);
    connect(m_versionList, &QListWidget::customContextMenuRequested, this, &MainWindow::onVersionContextMenu);
    connect(m_versionList, &QListWidget::itemDoubleClicked, this, &MainWindow::onVersionDoubleClicked);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::filterHistory);

    connect(m_storageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        const auto newType = static_cast<StorageType>(m_storageCombo->itemData(index).toInt());
        if (newType == m_controller->currentStorage())
            return;
        const auto reply = QMessageBox::question(this, "Зміна сховища", "Скопіювати поточні версії у нове сховище?",
                                                 QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) {
            m_storageCombo->setCurrentIndex(m_controller->currentStorage() == StorageType::File ? 0 : 1);
            return;
        }
        m_controller->handleSwitchStorage(newType);
    });

    connect(m_controller, &EditorController::versionAdded, this, &MainWindow::onVersionAdded);
    connect(m_controller, &EditorController::historyCleared, this, &MainWindow::onHistoryCleared);
    connect(m_controller, &EditorController::modifiedStateChanged, this, &MainWindow::onModifiedStateChanged);
    connect(m_controller, &EditorController::undoAvailableChanged, this, &MainWindow::onUndoAvailabilityChanged);
    connect(m_controller, &EditorController::redoAvailableChanged, this, &MainWindow::onRedoAvailabilityChanged);
    connect(m_controller, &EditorController::errorOccurred, this, &MainWindow::onErrorOccurred);
    connect(m_controller, &EditorController::historyReloaded, this, &MainWindow::onHistoryReloaded);
    connect(m_controller, &EditorController::storageChanged, this, &MainWindow::onStorageChanged);
    connect(m_controller, &EditorController::contentChanged, this, [this](const QString &c) {
        m_suppressTextSignal = true;
        m_textEdit->setPlainText(c);
        m_suppressTextSignal = false;
    });

    m_autosaveTimer = new QTimer(this);
    connect(m_autosaveTimer, &QTimer::timeout, this, &MainWindow::saveDraft);
    m_autosaveTimer->start(30000);

    const QString docsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/BranchDocs";
    std::filesystem::path draftPath = std::filesystem::path(docsPath.toStdString()) / "draft.txt";

    if (std::filesystem::exists(draftPath) && std::filesystem::file_size(draftPath) > 0) {
        auto reply = QMessageBox::question(this, "Відновлення", "Знайдено збережену чернетку. Бажаєте відновити її?",
                                           QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            QFile file(QString::fromStdString(draftPath.string()));
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                QString content = in.readAll();
                m_suppressTextSignal = true;
                m_textEdit->setPlainText(content);
                m_suppressTextSignal = false;
                m_controller->handleEdit(content);
            }
        }
    } else {
        onNewDocumentClicked();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveDraft();
    event->accept();
}

void MainWindow::saveDraft()
{
    if (!m_controller->document().isModified())
        return;

    const QString docsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/BranchDocs";
    std::filesystem::path draftPath = std::filesystem::path(docsPath.toStdString()) / "draft.txt";
    std::filesystem::create_directories(draftPath.parent_path());

    QFile file(QString::fromStdString(draftPath.string()));
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << m_textEdit->toPlainText();
        file.close();
        m_statusLabel->setText("Чернетку збережено локально (autosave)");
    }
}

void MainWindow::onStatsClicked()
{
    const auto &history = m_controller->document().history();
    size_t count = history.size();
    size_t totalSize = 0;
    for (const auto &v : history.getAllVersions()) {
        totalSize += v.content().size();
    }

    QMessageBox::information(this, "Статистика історії",
                             QString("Кількість версій: %1\nЗагальний обсяг тексту: %2 байт").arg(count).arg(totalSize));
}

void MainWindow::onVersionDoubleClicked(QListWidgetItem *item)
{
    if (!item)
        return;
    const QString versionId = item->data(Qt::UserRole).toString();
    auto &versions = m_controller->document().history().getAllVersions();
    for (const auto &v : versions) {
        if (QString::fromStdString(v.id()) == versionId) {
            VersionViewerDialog dialog(QString("Перегляд версії %1").arg(versionId.left(8)), QString::fromStdString(v.content()),
                                       this);
            dialog.exec();
            break;
        }
    }
}

void MainWindow::onVersionContextMenu(const QPoint &pos)
{
    auto *item = m_versionList->itemAt(pos);
    if (!item)
        return;
    const QString versionId = item->data(Qt::UserRole).toString();

    QMenu menu(this);
    auto *viewAction = menu.addAction("Переглянути вміст");
    auto *metaAction = menu.addAction("Метадані версії");
    menu.addSeparator();
    auto *restoreAction = menu.addAction("Відновити до цієї версії");
    auto *exportAction = menu.addAction("Експортувати у файл");
    menu.addSeparator();
    auto *deleteAction = menu.addAction("Видалити версію");

    QAction *chosen = menu.exec(m_versionList->mapToGlobal(pos));

    if (chosen == viewAction) {
        onVersionDoubleClicked(item);
    } else if (chosen == metaAction) {
        const domain::Version *v = m_controller->document().history().findById(versionId.toStdString());
        if (v) {
            QMessageBox::information(this, "Метадані версії",
                                     QString("ID: %1\nДата: %2\nКоментар: %3\nРозмір: %4 байт")
                                         .arg(QString::fromStdString(v->id()))
                                         .arg(QString::fromStdString(v->formattedTimestamp()))
                                         .arg(QString::fromStdString(v->comment()))
                                         .arg(v->content().size()));
        }
    } else if (chosen == restoreAction) {
        m_controller->handleRestore(versionId);
    } else if (chosen == exportAction) {
        const QString path = QFileDialog::getSaveFileName(this, "Експорт версії", "", "Text Files (*.txt)");
        if (!path.isEmpty())
            m_controller->handleExportVersion(versionId, path);
    } else if (chosen == deleteAction) {
        if (QMessageBox::question(this, "Видалення", "Ви впевнені, що хочете видалити цю версію?",
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            m_controller->handleDeleteVersion(versionId);
        }
    }
}

void MainWindow::onTextChanged()
{
    if (m_suppressTextSignal)
        return;
    m_controller->handleEdit(m_textEdit->toPlainText());
}

void MainWindow::onCommitClicked()
{
    bool ok = false;
    const QString comment = QInputDialog::getText(this, "Коментар", "Опишіть зміни:", QLineEdit::Normal, QString(), &ok);
    if (!ok)
        return;
    m_controller->handleCommit(comment);
}

void MainWindow::onNewDocumentClicked()
{
    bool ok = false;
    const QString title = QInputDialog::getText(this, "Новий документ", "Заголовок:", QLineEdit::Normal, "Без назви", &ok);
    if (!ok)
        return;
    m_controller->handleNewDocument(title);
}

void MainWindow::onOpenDocumentClicked()
{
    auto docs = m_controller->getAllDocuments();
    if (docs.empty()) {
        QMessageBox::information(this, "Відкрити", "Не знайдено жодного збереженого документа.");
        return;
    }

    QStringList items;
    for (const auto &doc : docs) {
        items << QString("%1 [%2]").arg(QString::fromStdString(doc.second), QString::fromStdString(doc.first).left(8));
    }

    bool ok = false;
    const QString selected = QInputDialog::getItem(this, "Відкрити документ", "Оберіть документ:", items, 0, false, &ok);
    if (!ok || selected.isEmpty())
        return;

    int idx = items.indexOf(selected);
    if (idx >= 0) {
        m_controller->handleOpenDocument(QString::fromStdString(docs[idx].first), QString::fromStdString(docs[idx].second));
        m_statusLabel->setText(QString("Відкрито: %1").arg(QString::fromStdString(docs[idx].second)));
    }
}

void MainWindow::onUndoClicked() { m_controller->handleUndo(); }
void MainWindow::onRedoClicked() { m_controller->handleRedo(); }

void MainWindow::onMergeClicked()
{
    const auto &versions = m_controller->document().history().getAllVersions();
    if (versions.size() < 2) {
        QMessageBox::information(this, "Merge", "Потрібно мінімум 2 версії.");
        return;
    }

    std::vector<MergeVersionItem> items;
    for (const auto &v : versions) {
        items.push_back({QString::fromStdString(v.id()),
                         QString("[%1] %2 — %3")
                             .arg(QString::fromStdString(v.id()).left(8), QString::fromStdString(v.formattedTimestamp()),
                                  v.comment().empty() ? "(без коментаря)" : QString::fromStdString(v.comment())),
                         QString::fromStdString(v.content())});
    }

    MergeDialog dialog(items, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    const QString merged = dialog.mergedText();
    if (merged.isEmpty())
        return;

    bool ok = false;
    const QString comment = QInputDialog::getText(this, "Коментар", "Опишіть об'єднання:", QLineEdit::Normal, "merge", &ok);
    if (!ok)
        return;
    m_controller->handleMergeCommit(merged, comment);
}

void MainWindow::onDiffClicked()
{
    auto selected = m_versionList->selectedItems();
    if (selected.size() != 2) {
        QMessageBox::information(this, "Diff", "Виберіть рівно дві версії (Ctrl+Click) для порівняння.");
        return;
    }

    auto &versions = m_controller->document().history().getAllVersions();
    QString id1 = selected[0]->data(Qt::UserRole).toString();
    QString id2 = selected[1]->data(Qt::UserRole).toString();

    QString c1, c2;
    for (const auto &v : versions) {
        if (QString::fromStdString(v.id()) == id1)
            c1 = QString::fromStdString(v.content());
        if (QString::fromStdString(v.id()) == id2)
            c2 = QString::fromStdString(v.content());
    }

    DiffDialog dialog(c1, c2, this);
    dialog.exec();
}

void MainWindow::onVersionAdded(const QString &id, const QString &timestamp, const QString &comment)
{
    const QString text = QString("[%1]  %2  —  %3").arg(id.left(8), timestamp, comment.isEmpty() ? "(без коментаря)" : comment);
    auto *item = new QListWidgetItem(text, m_versionList);
    item->setData(Qt::UserRole, id);
    m_statusLabel->setText(QString("Додано версію %1").arg(id.left(8)));
}

void MainWindow::onHistoryCleared()
{
    m_versionList->clear();
    m_statusLabel->setText("Створено новий документ");
}

void MainWindow::onModifiedStateChanged(bool modified)
{
    m_commitButton->setEnabled(modified);
    const QString title = QString::fromStdString(m_controller->document().title());
    setWindowTitle(QString("BranchDocs — %1%2").arg(title, modified ? " *" : ""));
}

void MainWindow::onUndoAvailabilityChanged(bool available) { m_undoButton->setEnabled(available); }
void MainWindow::onRedoAvailabilityChanged(bool available) { m_redoButton->setEnabled(available); }
void MainWindow::onErrorOccurred(const QString &message) { QMessageBox::warning(this, "Помилка", message); }
void MainWindow::onHistoryReloaded() { m_versionList->clear(); }

void MainWindow::onStorageChanged(StorageType type)
{
    m_storageCombo->setCurrentIndex(type == StorageType::File ? 0 : 1);
    m_statusLabel->setText(QString("Сховище: %1").arg(type == StorageType::File ? "JSON-файли" : "SQLite"));
}

void MainWindow::filterHistory(const QString &text)
{
    for (int i = 0; i < m_versionList->count(); ++i) {
        auto *item = m_versionList->item(i);
        item->setHidden(!item->text().contains(text, Qt::CaseInsensitive));
    }
}

} // namespace branchdocs