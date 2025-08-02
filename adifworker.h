#ifndef ADIFWORKER_H
#define ADIFWORKER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QString>

class AdifWorker : public QObject {
    Q_OBJECT

public:
    explicit AdifWorker(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void processFile(const QString &filePath);

signals:
    void finished(const QList<QMap<QString, QString>> &records);
    void error(QString message);
};

#endif // ADIFWORKER_H
