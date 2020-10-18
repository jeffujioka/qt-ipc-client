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

/// \details  IPC Protocol for getting data is described below:
///           Client: send 8-bit data (IpcProtReqId::kIpcProtReqIdStore, decimal 156)
///           Client: send bytesToSend bytes (in chunks of kIpcMaxPayloadLength)
///           of dummy data to the Server.
///           Client: receive 32-bit data from Server.
///                   Current implementation only supports receiving
///                   kIpcProtAck (0xdeadbeef) but Server could respond with
///                   some other Error Code.
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
    // fill buffer with dummy ASCII data
    fillWithASCII(buffer, kIpcMaxPayloadLength);

    while (remainingBytes > 0) {
        auto bytesToWrite = qMin(remainingBytes,
                                 static_cast<qint64>(kIpcMaxPayloadLength));

        // write up to kIpcMaxPayloadLength to the Server
        qint64 bytesWritten = socket.write(buffer, bytesToWrite);

        if (bytesWritten < 0) {
            if (!socket.waitForBytesWritten()) {
                QString err{"Error writing data"};
                qDebug() << err;
                emit error(err);
                if (buffer) delete[] buffer;
                return;
            }
        }
        socket.flush();

        remainingBytes -= bytesWritten;
    }
    socket.flush();
    if (buffer) delete[] buffer;
    // no error has occured so let's wait for Server's ACK
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
