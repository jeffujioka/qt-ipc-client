#ifndef IPCCLIENT_H
#define IPCCLIENT_H

#include <QObject>
#include <QString>

class IpcClient : public QObject
{
    Q_OBJECT
public:
    IpcClient(QObject *parent = nullptr);

public slots:
    void sendData(const QString& srvName, int bytes);
    void getData(const QString& srvName, const QString& fileName);

private slots:
    void errorString(QString err);
    void threadFinished();

signals:
    void getFinished();
    void sendFinished();
    void error();

};

#endif // IPCCLIENT_H
