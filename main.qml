import QtQuick 2.13
import QtQuick.Extras 1.4
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3
import QtQuick.Window 2.13

import de.com.fujioka 1.0

Window {
    id: root
    width: 640
    height: 480
    visible: true
    title: qsTr("IPC Client")

    IpcClient {
        id: ipcClient;

        onSendFinished: {
            sendButton.enabled = true
            statusSendLbl.text = "Data has been successfully sent!"
            statusSendIndicator.color = "green"
        }

        onGetFinished: {
            getButton.enabled = true
            statusGetLbl.text = "Data has been successfully received!"
            statusGetIndicator.color = "green"
        }

        onError: {
            sendButton.enabled = true
            statusSendLbl.text = "Fail to send data!"
            statusSendIndicator.color = "red"
        }
    }

    ColumnLayout {
        id: mainLayout
        visible: true

        GroupBox {
            id: gridBox

            GridLayout {
                id: gridLayout
                rows: 2
                flow: GridLayout.TopToBottom
                anchors.fill: parent

                Label {
                    id: srvNameLbl
                    text: "Server name"
                }
                Label {
                    id: numBytesLbl
                    text: "Number of bytes to send"
                }

                TextField {
                    id: srvNameTxt
                    text: "mbition"
                }
                TextField {
                    id: numBytesTxt
                    text: "1024"
                }
            }
        }

        GroupBox {
            id: buttonBox

            Button {
                id: sendButton
                text: "send"

                MessageDialog {
                    id: msgDial
                    icon: StandardIcon.Critical
                    standardButtons: MessageDialog.Ok
                }

                onClicked: {
                    if (srvNameTxt.text == "") {
                        console.log("Server Name is empty")
                        msgDial.text = "Server Name is required!"
                        msgDial.open()
                    } else if (isNaN(numBytesTxt.text)) {
                        console.log(numBytesTxt.text + " is not a number")
                        msgDial.text = numBytesTxt.text + " is not a number"
                        msgDial.open()
                    }
                    else {
                        sendButton.enabled = false
                        statusSendIndicator.color = "blue"
                        statusSendLbl.text = "Idle"
                        ipcClient.sendData(srvNameTxt.text,
                                           parseInt(numBytesTxt.text))
                    }
                }
            }
        }

        GroupBox {
            GridLayout {
                rows: 1
                flow: GridLayout.TopToBottom
                anchors.fill: parent

                StatusIndicator {
                    id: statusSendIndicator
                    color: "blue"
                    active: true
                }

                Label {
                    id: statusSendLbl
                    text: "Idle"
                }
            }
        }

        GroupBox {
            GridLayout {
                id: gridRecvLayout
                rows: 1
                columns: 2
                flow: GridLayout.TopToBottom
                anchors.fill: parent

                Label {
                    id: fileNameLbl
                    text: "file name: "
                }

                TextField {
                    id: fileNameTxt
                    text: "mbition.txt"
                }
            }
        }

        GroupBox {
            Button {
                id: getButton
                text: "get"

                MessageDialog {
                    id: getMsgDial
                    icon: StandardIcon.Critical
                    standardButtons: MessageDialog.Ok
                }

                onClicked: {
                    if (fileNameTxt.text == "") {
                        console.log("file name is empty")
                        getMsgDial.text = "file name is required!"
                        getMsgDial.open()
                    } else {
                        getButton.enabled = false
                        statusGetIndicator.color = "yellow"
                        statusGetLbl.text = "Processing"
                        ipcClient.getData(srvNameTxt.text,
                                          fileNameTxt.text)
                    }
                }
            }
        }

        GroupBox {
            GridLayout {
                rows: 1
                flow: GridLayout.TopToBottom
                anchors.fill: parent

                StatusIndicator {
                    id: statusGetIndicator
                    color: "blue"
                    active: true
                }

                Label {
                    id: statusGetLbl
                    text: "Idle"
                }
            }
        }

    }

}
