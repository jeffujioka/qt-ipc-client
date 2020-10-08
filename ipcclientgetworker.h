#ifndef IPCCLIENTGETWORKER_H
#define IPCCLIENTGETWORKER_H

#include <QDataStream>
#include <QFile>
#include <QLocalSocket>
#include <QObject>
#include <QString>
#include <QThread>

Q_DECLARE_METATYPE(QLocalSocket::LocalSocketError);

class IpcClientGetWorker : public QObject
{
    Q_OBJECT
public:
    IpcClientGetWorker(const QString& srvName, const QString& fname);

private:
    bool openFile(QIODevice::OpenMode openMode);
    void closeFile();

public slots:
    void process();
    void sendRequest();
    void sendAck();
    void disconected();

signals:
    void finished();
    void error(QString err);

private:
    QString srvName;
    QString fileName;
    QFile* file;
    uint bytesToSend;
    QLocalSocket socket;
    QDataStream in;
};

#endif // IPCCLIENTGETWORKER_H
