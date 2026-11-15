#include "DiffDialog.h"
#include <QColor>
#include <QDialogButtonBox>
#include <QFont>
#include <QHeaderView>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>

#include "SentenceSplitter.h"

namespace branchdocs {
DiffDialog::DiffDialog(const QString &versionA, const QString &versionB, QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Порівняння версій (Diff)");
    resize(1100, 700);

    auto *layout = new QVBoxLayout(this);
    m_table = new QTableWidget(this);

    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"№", "Версія A", "№", "Версія B"});

    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setWordWrap(true);

    QFont monoFont("Consolas", 10);
    monoFont.setStyleHint(QFont::Monospace);
    m_table->setFont(monoFont);

    QStringList sentencesA = splitIntoSentences(versionA);
    QStringList sentencesB = splitIntoSentences(versionB);
    int maxRows = static_cast<int>(std::max(sentencesA.size(), sentencesB.size()));
    m_table->setRowCount(maxRows);

    for (int i = 0; i < maxRows; ++i) {
        QString a = i < sentencesA.size() ? sentencesA[i] : "";
        QString b = i < sentencesB.size() ? sentencesB[i] : "";

        auto *numA = new QTableWidgetItem(i < sentencesA.size() ? QString::number(i + 1) : "");
        auto *numB = new QTableWidgetItem(i < sentencesB.size() ? QString::number(i + 1) : "");

        auto *itemA = new QTableWidgetItem(a);
        auto *itemB = new QTableWidgetItem(b);

        numA->setTextAlignment(Qt::AlignRight | Qt::AlignTop);
        numB->setTextAlignment(Qt::AlignRight | Qt::AlignTop);

        QColor textColor("#000000");
        numA->setForeground(textColor);
        numB->setForeground(textColor);
        itemA->setForeground(textColor);
        itemB->setForeground(textColor);

        if (a == b) {
            itemA->setBackground(QColor("#f0f0f0"));
            itemB->setBackground(QColor("#f0f0f0"));
        } else {
            if (!a.isEmpty())
                itemA->setBackground(QColor("#ffcccc"));
            if (!b.isEmpty())
                itemB->setBackground(QColor("#ccffcc"));
        }

        m_table->setItem(i, 0, numA);
        m_table->setItem(i, 1, itemA);
        m_table->setItem(i, 2, numB);
        m_table->setItem(i, 3, itemB);

        int height = static_cast<int>(std::max(a.length(), b.length()) / 1.5);
        m_table->setRowHeight(i, std::max(30, height));
    }

    layout->addWidget(m_table);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
} // namespace branchdocs