#include "ipcclient.h"

#include <QDebug>

#include "ipcclientsendworker.h"
#include "ipcclientgetworker.h"

IpcClient::IpcClient(QObject *parent)
    : QObject(parent)
{

}

// FIXME: refactor: move it to an "util" header.
/// \brief   Helper function to create a new Thread.
/// \details Creates a new Thread then move \a worker to it.
///          Concept for Worker typename
/// \code{.cpp}
/// class Worker : public {
/// signals:
///     void finished();
///     void error();
/// };
/// \endcode
///          Worker::finished
template <typename Worker>
QThread* newThread(Worker* worker) {
    QThread* thread = new QThread;
    worker->moveToThread(thread);

    QObject::connect(thread, &QThread::started,
                     worker, &Worker::process);
    QObject::connect(worker, &Worker::finished,
                     thread, &QThread::quit);
    QObject::connect(worker, &Worker::finished,
                     worker, &Worker::deleteLater);
    QObject::connect(thread, &QThread::finished,
                     thread, &QThread::deleteLater);

    return thread;
}

void IpcClient::sendData(const QString& srvName, int bytes) {
    qDebug() << "sending [" << bytes << "] bytes to [" << srvName << "]";

    // Initialy I had created an IpcClientThread (inhireted from QThread)
    // then I was trying to start it here but I was getting the following error:
    // QObject: Cannot create children for a parent that is in a different thread
    // so I googled it and found this: https://wiki.qt.io/QThreads_general_usage.
    // but it also didn't work and I am still getting the same error.

    auto worker = new IpcClientSendWorker(srvName, bytes);
    QThread* thread = newThread<IpcClientSendWorker>(worker);

    // connecting worker's error signal to IpcClient::error
    QObject::connect(worker, &IpcClientSendWorker::error,
                     this, &IpcClient::error);
    // connecting QThread'::'s finished signal to IpcClient::sendFinished
    QObject::connect(thread, &QThread::finished,
                     this, &IpcClient::sendFinished);

    thread->start();
}

void IpcClient::getData(const QString& srvName, const QString& fileName) {
    qDebug() << "getData " << srvName << " " << fileName;

    auto worker = new IpcClientGetWorker(srvName, fileName);
    QThread* thread = newThread<IpcClientGetWorker>(worker);

    // connecting worker's error signal to IpcClient::error
    QObject::connect(worker, &IpcClientGetWorker::error,
                     this, &IpcClient::error);
    // connecting QThread'::'s finished signal to IpcClient::getFinished
    QObject::connect(thread, &QThread::finished,
                     this, &IpcClient::getFinished);

    thread->start();
}
