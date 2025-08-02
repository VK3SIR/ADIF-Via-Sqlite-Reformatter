#pragma once

#ifndef ADIFPARSER_H
#define ADIFPARSER_H

#pragma once

#include <QString>
#include <QList>
#include <QMap>

class AdifParser {
public:
    QList<QMap<QString, QString>> parseFile(const QString &filePath);

private:
    QMap<QString, QString> parseRecord(const QString &recordText);
};
#endif // ADIFPARSER_H
