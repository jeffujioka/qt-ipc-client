#ifndef TESTCLASS_H
#define TESTCLASS_H

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


#endif // TESTCLASS_H
