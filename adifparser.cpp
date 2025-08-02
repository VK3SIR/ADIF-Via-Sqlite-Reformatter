#include "adifparser.h"
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

QList<QMap<QString, QString>> AdifParser::parseFile(const QString &filePath) {
    QFile file(filePath);
    QList<QMap<QString, QString>> records;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return records;

    QTextStream in(&file);
    QString content = in.readAll();
    content.replace("\r\n", "\n");
    content.replace("<eor>", "<EOR>");      // Just to be safe

    //const QStringList rawRecords = content.split("<\n>", Qt::SkipEmptyParts);
    //const QStringList
    auto rawRecords = content.split("<EOR>", Qt::SkipEmptyParts);

    for (const QString &recordText : rawRecords) {
        QMap<QString, QString> record = parseRecord(recordText);
        if (!record.isEmpty())
            records.append(record);
    }

    return records;
}

QMap<QString, QString> AdifParser::parseRecord(const QString &recordText) {
    QMap<QString, QString> record;
    QRegularExpression rx("<(\\w+):([0-9]+)>([^<]*)", QRegularExpression::CaseInsensitiveOption);

    auto matches = rx.globalMatch(recordText);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        QString field = match.captured(1).toUpper();
        int len = match.captured(2).toInt();
        QString value = match.captured(3).left(len).trimmed();
        record[field] = value;
    }

    return record;
}
