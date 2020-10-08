#include "ipcclientworker.h"

#include "ipccommon.h"

IpcClientWorker::IpcClientWorker(const QString& srvName)
    : srvName(srvName) {
//    qRegisterMetaType<QAbstractSocket::SocketState>();
//    qRegisterMetaType<QLocalSocket::LocalSocketError>();

    in.setDevice(&socket);
    in.setVersion(QDataStream::Qt_5_10);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);

    connect(&socket, &QLocalSocket::errorOccurred,
            [this](QLocalSocket::LocalSocketError err) {
                qDebug() << "lambda: " << err;
                emit error("");
            });
    connect(&socket, &QLocalSocket::connected,
            this, &IpcClientWorker::sendRequest);
}

void IpcClientWorker::sendAck() {
    out << kIpcProtAck;

    socket.write(block);
    socket.flush();
}

void IpcClientWorker::waitForAck() {
    if (!socket.waitForReadyRead()) {
        qDebug() << "Error waiting for ReadyRead!";
        emit error(socket.errorString());
    }

    quint32 ack;

    in >> ack;

    if (ack != kIpcProtAck) {
        QString err = "Invalid [";
                err += ack;
                err += "] ACK. Something went wrong.";
        qDebug() << err;
        emit error(err);
    }
}
