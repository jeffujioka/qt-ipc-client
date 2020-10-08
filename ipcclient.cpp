#include "ipcclient.h"

#include <QDebug>

#include "ipcclientsendworker.h"
#include "ipcclientgetworker.h"

IpcClient::IpcClient(QObject *parent)
    : QObject(parent)
{

}

void IpcClient::sendData(const QString& srvName, int bytes) {
    qDebug() << "sending [" << bytes << "] bytes to [" << srvName << "]";

    // Initialy I had created an IpcClientThread (inhireted from QThread)
    // then I was trying to start it here but I was getting the following error:
    // QObject: Cannot create children for a parent that is in a different thread
    // so I googled it and found this: https://wiki.qt.io/QThreads_general_usage
    QThread* thread = new QThread;
    IpcClientSendWorker * worker = new IpcClientSendWorker(srvName, bytes);
    worker->moveToThread(thread);

    connect(worker, &IpcClientSendWorker::error, this, ([this](QString err)
            {
                qDebug() << "Error: " << err;
                emit error();
            }));
    connect(thread, &QThread::started,
            worker, &IpcClientSendWorker::process);
    connect(worker, &IpcClientSendWorker::finished,
            thread, &QThread::quit);
    connect(worker, &IpcClientSendWorker::finished,
            worker, &IpcClientSendWorker::deleteLater);
    connect(thread, &QThread::finished, ([this]()
            {
                emit sendFinished();
            }));
    connect(thread, &QThread::finished,
            thread, &QThread::deleteLater);

    thread->start();
}

void IpcClient::getData(const QString& srvName, const QString& fileName) {
    qDebug() << "getData " << srvName << " " << fileName;
    QThread* thread = new QThread;
    IpcClientGetWorker * worker = new IpcClientGetWorker(srvName, fileName);
    worker->moveToThread(thread);

    connect(worker, &IpcClientGetWorker::error, this, ([this](QString err)
            {
                qDebug() << "Error: " << err;
                emit error();
            }));
    connect(thread, &QThread::started,
            worker, &IpcClientGetWorker::process);
    connect(worker, &IpcClientGetWorker::finished,
            thread, &QThread::quit);
    connect(worker, &IpcClientGetWorker::finished,
            worker, &IpcClientGetWorker::deleteLater);
    connect(thread, &QThread::finished, ([this]()
            {
                emit getFinished();
            }));
    connect(thread, &QThread::finished,
            thread, &QThread::deleteLater);

    thread->start();
}

void IpcClient::errorString(QString err) {
    qDebug() << "Error: " << err;
    emit error();
}

void IpcClient::threadFinished() {
    qDebug() << "IpcClient finished!!!";
    emit getFinished();
}

