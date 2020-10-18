#include "ipcclientgetworker.h"

#include <QDebug>

#include "ipccommon.h"

IpcClientGetWorker::IpcClientGetWorker(const QString& srvName,
                                       const QString& fname)
    : srvName(srvName), fileName(fname), file(nullptr) {
    qRegisterMetaType<QAbstractSocket::SocketState>();
    qRegisterMetaType<QLocalSocket::LocalSocketError>();

    in.setDevice(&socket);
    in.setVersion(QDataStream::Qt_5_10);

    connect(&socket, &QLocalSocket::errorOccurred,
            [](QLocalSocket::LocalSocketError err) {
                qDebug() << "IpcClientGetWorker error: " << err;
            });
    connect(&socket, &QLocalSocket::disconnected,
            []() {
                qDebug() << "IpcClientGetWorker Disconnected!!!";
            });
    connect(&socket, &QLocalSocket::connected,
            this, &IpcClientGetWorker::sendRequest);
}

void IpcClientGetWorker::process() {
    qDebug() << "IpcClientGetWorker connecting to: " << srvName;
    socket.abort();
    socket.connectToServer(srvName);
}

/// \details  IPC Protocol for getting data is described below:
///           Client: send 8-bit data (IpcProtReqId::kIpcProtReqIdGet, decimal 157)
///           Client: receive 64-bit data representing the number of bytes
///                   to received from Server.
///           Client: send 32-bit data (kIpcProtAck, 0xdeadbeef) to Server.
///                   Current implementation only supports sending
///                   kIpcProtAck (0xdeadbeef) but Server could treat other
///                   Error Codes (e.g. re-send last packet).
void IpcClientGetWorker::sendRequest() {
    qDebug() << "Connected to [" << srvName << "]";

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);

    out << IpcProtReqId::kIpcProtReqIdGet;

    socket.write(block);

    if (!socket.waitForReadyRead()) {
        qDebug() << "Error: " << socket.errorString();
        emit error(socket.errorString());
        return;
    }

    qint64 totalBytesToReceiveFromServer;

    // read 64-bit from Server through socket.
    in >> totalBytesToReceiveFromServer;

    uint totalReadBytes = 0;

    QString errMsg;

    // opens the file to store the data received from Server.
    if (!openFile(QIODevice::WriteOnly
                  | QIODevice::Text | QIODevice::Truncate)) {
        errMsg.append("Error opening file ")
              .append(file->fileName());
        qDebug() <<  errMsg;
        goto exit;
    }

    while (totalReadBytes < totalBytesToReceiveFromServer)
    {
        char data[kIpcMaxPayloadLength];

        // reads in chunks of kIpcMaxPayloadLength
        auto readBytes = socket.read(data, kIpcMaxPayloadLength);

        if (readBytes < 0) {
            qDebug() << "needs more data";
            if (!socket.waitForReadyRead(3000)) {
                errMsg.append("error waiting more data: ")
                      .append(socket.errorString());
                goto exit;
            }
        }

        totalReadBytes += readBytes;

        // writes readBytes bytes read from Server to the file
        file->write(data, readBytes);

        if (totalReadBytes < totalBytesToReceiveFromServer && in.atEnd()) {
            // there no enough data so let's wait the server to send more
            if (!socket.waitForReadyRead()) {
                errMsg.append("error waiting more data: ")
                      .append(socket.errorString());
                goto exit;
            }
        }
    }

    sendAck();
    qDebug() << "//////////////////////"
             << "totalReadBytes: " << totalReadBytes;

    exit:
    closeFile();

    if (!errMsg.isEmpty()) {
        qDebug() << errMsg;
        emit error(errMsg); // notifies an error
    } else {
        emit finished(); // notifies the end of the worker's processing
    }
}

void IpcClientGetWorker::sendAck() {
    // there is no need to use QDataStream to encode ACK but I started using it
    // and had no time to change it.
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);

    out << kIpcProtAck;

    socket.write(block);
    socket.flush();
}

void IpcClientGetWorker::disconected() {
    qDebug() << "get worker Disconnected!!!";
}

bool IpcClientGetWorker::openFile(QIODevice::OpenMode openMode) {
    if (!file) file = new QFile(fileName);

    return file->open(openMode);
}

void IpcClientGetWorker::closeFile() {
    if (file && file->isOpen()) file->close();
    file = nullptr;
}
