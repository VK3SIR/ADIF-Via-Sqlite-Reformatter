#include "adifworker.h"
#include "adifparser.h" // assumes parseFile is in this class

void AdifWorker::processFile(const QString &filePath) {
    AdifParser parser;
    auto records = parser.parseFile(filePath);
    emit finished(records);
}
