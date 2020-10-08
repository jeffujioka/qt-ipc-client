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

    qint64 fileSize;

    in >> fileSize;

    uint totalReadBytes = 0;

    QString errMsg;

    if (!openFile(QIODevice::WriteOnly
                  | QIODevice::Text | QIODevice::Truncate)) {
        errMsg.append("Error opening file ")
              .append(file->fileName());
        qDebug() <<  errMsg;
        goto exit;
    }

    while (totalReadBytes < fileSize)
    {
        char data[kIpcMaxPayloadLength];

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
#if 0
        qDebug() << "Writing " << readBytes
                 << "/" << totalReadBytes << " of " << fileSize;
#endif
        file->write(data, readBytes);

        if (totalReadBytes < fileSize && in.atEnd()) {
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
        emit error(errMsg);
    } else {
        emit finished();
    }
}

void IpcClientGetWorker::sendAck() {
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
