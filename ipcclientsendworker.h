#ifndef IPCCLIENTWORKER_H
#define IPCCLIENTWORKER_H

#include <QDataStream>
#include <QLocalSocket>
#include <QObject>
#include <QString>
#include <QThread>

class IpcClientSendWorker : public QObject
{
    Q_OBJECT
public:
    IpcClientSendWorker(const QString& srvName, int bytes);

private:
    void fillWithASCII(char* buffer, quint32 length);
    void waitForAck();

public slots:
    void process();
    void sendRequest();
    void disconected();

signals:
    void finished();
    void error(QString err);

private:
    QString srvName;
    quint32 bytesToSend;
    QLocalSocket socket;
    QDataStream in;
};

#endif // IPCCLIENTWORKER_H
