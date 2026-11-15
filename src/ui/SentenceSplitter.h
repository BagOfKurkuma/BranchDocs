#pragma once
#include <QRegularExpression>
#include <QString>
#include <QStringList>

namespace branchdocs {
inline QStringList splitIntoSentences(const QString &text)
{
    QRegularExpression re("(?<=[.!?])\\s+|\\n+");
    QStringList result = text.split(re, Qt::SkipEmptyParts);
    for (QString &s : result)
        s = s.trimmed();
    result.removeAll("");
    return result;
}
} // namespace branchdocs