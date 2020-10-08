#include "ipcclientsendworker.h"

#include <QtGlobal>
#include <QThread>

#include "ipccommon.h"

IpcClientSendWorker::IpcClientSendWorker(const QString& srvName, int bytes)
    : srvName(srvName), bytesToSend(bytes) {
    qRegisterMetaType<QAbstractSocket::SocketState>();

    in.setDevice(&socket);
    in.setVersion(QDataStream::Qt_5_10);

    connect(&socket, &QLocalSocket::errorOccurred,
            [](QLocalSocket::LocalSocketError err)
            {
                qDebug() << "send worker lambda: " << err;
            });
    connect(&socket, &QLocalSocket::connected,
            this, &IpcClientSendWorker::sendRequest);
    connect(&socket, &QLocalSocket::disconnected,
            this, &IpcClientSendWorker::disconected);
}

void IpcClientSendWorker::process() {
    qDebug() << "IpcClientSendWorker connecting to [" << srvName << "]";
    socket.abort();
    socket.connectToServer(srvName);
}

void IpcClientSendWorker::sendRequest() {
    qDebug() << "IpcClientSendWorker connected to [" << srvName << "]";

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);

    out << IpcProtReqId::kIpcProtReqIdStore;
    out << bytesToSend;

    socket.write(block);

    qint64 remainingBytes = bytesToSend;

    char *buffer = new char[kIpcMaxPayloadLength];
    if (!buffer) {
        QString err{"Error allocating memory"};
        qDebug() << err;
        emit error(err);
        return;
    }
    fillWithASCII(buffer, kIpcMaxPayloadLength);

    while (remainingBytes > 0) {
        auto bytesToWrite = qMin(remainingBytes,
                                 static_cast<qint64>(kIpcMaxPayloadLength));

        qint64 bytesWritten = socket.write(buffer, bytesToWrite);

        if (bytesWritten < 0) {
            if (!socket.waitForBytesWritten()) {
                QString err{"Error writing data"};
                qDebug() << err;
                emit error(err);
                return;
            }
        }
        socket.flush();

        remainingBytes -= bytesWritten;
    }
    socket.flush();
    if (buffer) delete[] buffer;

    waitForAck();
}

void IpcClientSendWorker::waitForAck() {
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
    socket.disconnectFromServer();
    emit finished();
}

void IpcClientSendWorker::disconected() {
    qDebug() << "Disconnected!!!";
}

void IpcClientSendWorker::fillWithASCII(char* buffer, quint32 length) {
    quint8 ch = 32;
    for (auto i = 0u; i < length; ++i) {
        buffer[i] = ch;
        if (++ch == 126) ch = 32;
    }
}
