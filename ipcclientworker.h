#ifndef TESTCLASS_H
#define TESTCLASS_H

// FIXME IpcClientGetWorker and IpcClientSendWorker shall inherit from it
#if 0
#include <QDataStream>
#include <QFile>
#include <QLocalSocket>
#include <QObject>
#include <QString>
#include <QThread>

//Q_DECLARE_METATYPE(QLocalSocket::LocalSocketError);

class IpcClientWorker : public QObject
{
   Q_OBJECT
public:
   IpcClientWorker(const QString& srvName);

public slots:
   virtual void process() = 0;
   virtual void sendRequest() = 0;

protected:
   void sendAck();
   void waitForAck();

signals:
   void error(QString err);

protected:
   QString srvName;
   QLocalSocket socket;
   QDataStream in;
   QDataStream out;
   QByteArray block;
};
#endif

#endif // TESTCLASS_H
