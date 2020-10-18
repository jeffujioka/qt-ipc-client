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
    /// \brief     Handles the "send data request" received from GUI.
    /// \details   Creates and starts a new thread which will run the
    ///            IpcClientSendWorker.
    ///            IpcClientSendWorker will connect to the Serve's socket
    ///            \a srvName and send \a bytes bytes (dummy generated data)
    ///            through it.
    /// \param[in] srvName   The name of the Server's socket
    /// \param[in] bytes     The number of bytes to be sent to the Server.
    void sendData(const QString& srvName, int bytes);

    /// \brief     Handles the "get data request" received from GUI.
    /// \details   Creates and starts a new thread which
    ///            will run the IpcClientGetWorker.
    ///            IpcClientGetWorker will connect to the Serve's socket
    ///            \a srvName and get data through it.
    /// \param[in] srvName   The name of the Server's socket
    /// \param[in] fileName  The name of the file to store the data received
    ///                      from the Server.
    void getData(const QString& srvName, const QString& fileName);

signals:
    /// \brief Signal to notify GUI that "get data request" has been finished.
    void getFinished();

    /// \brief Signal to notify GUI that "send data request" has been finished.
    void sendFinished();

    /// \brief Signal to notify GUI that an error has occured.
    void error();

};

#endif // IPCCLIENT_H
