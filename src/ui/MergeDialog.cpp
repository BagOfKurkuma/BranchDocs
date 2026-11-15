#include "MergeDialog.h"
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFont>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>

#include "SentenceSplitter.h"

namespace branchdocs {
MergeDialog::MergeDialog(const std::vector<MergeVersionItem> &versions, QWidget *parent) : QDialog(parent), m_versions(versions)
{
    setWindowTitle("Ручне об'єднання (Merge)");
    resize(1200, 800);

    auto *root = new QVBoxLayout(this);
    auto *top = new QHBoxLayout();

    top->addWidget(new QLabel("Версія A:", this));
    m_comboA = new QComboBox(this);
    top->addWidget(m_comboA, 1);
    top->addSpacing(24);
    top->addWidget(new QLabel("Версія B:", this));
    m_comboB = new QComboBox(this);
    top->addWidget(m_comboB, 1);
    root->addLayout(top);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({"Вибрати", "№", "Версія A", "№", "Версія B"});

    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);

    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setWordWrap(true);

    QFont monoFont("Consolas", 10);
    monoFont.setStyleHint(QFont::Monospace);
    m_table->setFont(monoFont);

    root->addWidget(m_table, 3);

    root->addWidget(new QLabel("Результат:", this));
    m_preview = new QPlainTextEdit(this);
    m_preview->setReadOnly(true);
    root->addWidget(m_preview, 2);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    root->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    for (std::size_t i = 0; i < m_versions.size(); ++i) {
        m_comboA->addItem(m_versions[i].label, static_cast<int>(i));
        m_comboB->addItem(m_versions[i].label, static_cast<int>(i));
    }
    if (m_versions.size() >= 2) {
        m_comboA->setCurrentIndex(static_cast<int>(m_versions.size()) - 2);
        m_comboB->setCurrentIndex(static_cast<int>(m_versions.size()) - 1);
    }

    connect(m_comboA, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MergeDialog::onVersionsChanged);
    connect(m_comboB, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MergeDialog::onVersionsChanged);

    onVersionsChanged();
}

void MergeDialog::onVersionsChanged()
{
    const int idxA = m_comboA->currentData().toInt();
    const int idxB = m_comboB->currentData().toInt();
    if (idxA < 0 || idxB < 0)
        return;

    QStringList sentencesA = splitIntoSentences(m_versions[idxA].content);
    QStringList sentencesB = splitIntoSentences(m_versions[idxB].content);
    int maxRows = static_cast<int>(std::max(sentencesA.size(), sentencesB.size()));

    m_table->clearContents();
    m_table->setRowCount(maxRows);

    QColor textColor("#000000");

    for (int i = 0; i < maxRows; ++i) {
        QString a = i < sentencesA.size() ? sentencesA[i] : "";
        QString b = i < sentencesB.size() ? sentencesB[i] : "";

        auto *numA = new QTableWidgetItem(i < sentencesA.size() ? QString::number(i + 1) : "");
        auto *numB = new QTableWidgetItem(i < sentencesB.size() ? QString::number(i + 1) : "");
        auto *itemA = new QTableWidgetItem(a);
        auto *itemB = new QTableWidgetItem(b);
        auto *cb = new QCheckBox(this);

        numA->setTextAlignment(Qt::AlignRight | Qt::AlignTop);
        numB->setTextAlignment(Qt::AlignRight | Qt::AlignTop);

        numA->setForeground(textColor);
        numB->setForeground(textColor);
        itemA->setForeground(textColor);
        itemB->setForeground(textColor);

        if (a == b) {
            cb->setEnabled(false);
            cb->setChecked(true);
            itemA->setBackground(QColor("#f0f0f0"));
            itemB->setBackground(QColor("#f0f0f0"));
        } else {
            if (a.isEmpty()) {
                cb->setChecked(false);
                itemB->setBackground(QColor("#ccffcc"));
            } else if (b.isEmpty()) {
                cb->setChecked(true);
                itemA->setBackground(QColor("#ffcccc"));
            } else {
                cb->setChecked(true);
                itemA->setBackground(QColor("#ffcccc"));
                itemB->setBackground(QColor("#ccffcc"));
            }
        }

        auto *cbWidget = new QWidget();
        auto *cbLayout = new QHBoxLayout(cbWidget);
        cbLayout->addWidget(cb);
        cbLayout->setAlignment(Qt::AlignCenter);
        cbLayout->setContentsMargins(0, 0, 0, 0);

        connect(cb, &QCheckBox::toggled, this, &MergeDialog::updatePreview);
        m_table->setCellWidget(i, 0, cbWidget);
        m_table->setItem(i, 1, numA);
        m_table->setItem(i, 2, itemA);
        m_table->setItem(i, 3, numB);
        m_table->setItem(i, 4, itemB);

        int height = static_cast<int>(std::max(a.length(), b.length()) / 1.5);
        m_table->setRowHeight(i, std::max(30, height));
    }
    updatePreview();
}

QString MergeDialog::assembleResult() const
{
    QStringList result;
    for (int i = 0; i < m_table->rowCount(); ++i) {
        auto *cbWidget = qobject_cast<QWidget *>(m_table->cellWidget(i, 0));
        if (!cbWidget)
            continue;
        auto *cb = cbWidget->findChild<QCheckBox *>();
        if (!cb)
            continue;

        QString textA = m_table->item(i, 2)->text();
        QString textB = m_table->item(i, 4)->text();

        QString selected = cb->isChecked() ? textA : textB;
        if (!selected.isEmpty()) {
            result << selected;
        }
    }
    return result.join(" ");
}

void MergeDialog::updatePreview() { m_preview->setPlainText(assembleResult()); }

QString MergeDialog::mergedText() const { return assembleResult(); }
} // namespace branchdocs