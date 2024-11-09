import QtQuick 2.15
import QtQuick.Controls 2.4

Page {
    id: pageConnect
    width: 400
    height: 700
    property alias listView: listView
    property alias mouseListView: mouseListView
    property alias groupBox: groupBox
    property alias groupBox1: groupBox1
    property alias progressBar: progressBar
    property alias bFirmwareStop: bFirmwareStop
    property alias label1: label1
    property alias bFirmwareStart: bFirmwareStart
    property alias bFirmware: bFirmware
    property alias bDisconnect: bDisconnect
    property alias bConnect: bConnect
    property alias sbPort: sbPort


    topPadding: 20
    layer.enabled: false

    title: qsTr("Page 1")

    background: Rectangle {
        anchors.fill: parent
        color: "lightgray"
    }

    GroupBox {
        id: groupBox

        height: 200
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.topMargin: 10
        title: qsTr("Подключение к устройству")

        SpinBox {
            id: sbPort
            height: 40
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: label.bottom
            anchors.topMargin: 5
            anchors.leftMargin: 0
            value: 33100
            to: 65535
            from: 1
            anchors.rightMargin: 0
        }

        Button {
            id: bConnect
            width: (parent.width / 2) - 20
            text: qsTr("Подключить")
            anchors.left: parent.left
            anchors.top: sbPort.bottom
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            topPadding: 12
            anchors.leftMargin: 0
            anchors.topMargin: 10
        }

        Button {
            id: bDisconnect
            text: qsTr("Отключить")
            anchors.left: bConnect.right
            anchors.right: parent.right
            anchors.top: sbPort.bottom
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.leftMargin: 20
            anchors.rightMargin: 0
            anchors.topMargin: 10
            enabled: false
        }

        Label {
            id: label
            text: qsTr("Номер порта")
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 10
        }
    }

    GroupBox {
        id: groupBox1
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: groupBox.bottom
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.topMargin: 20
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        title: qsTr("Прошивка")

        Button {
            id: bFirmware
            text: qsTr("Выбрать файл прошивки")
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: 0
            anchors.leftMargin: 0
            anchors.topMargin: 5
        }

        Label {
            id: label1
            text: qsTr("Файл прошивки")
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: bFirmware.bottom
            anchors.topMargin: 10
            anchors.leftMargin: 0
            anchors.rightMargin: 0
        }

        Button {
            id: bFirmwareStart
            width: (parent.width / 2) - 20
            height: 40
            text: qsTr("Старт")
            anchors.left: parent.left
            anchors.top: label1.bottom
            anchors.leftMargin: 0
            anchors.topMargin: 10
        }

        Button {
            id: bFirmwareStop
            height: 40
            text: qsTr("Стоп")
            anchors.left: bFirmwareStart.right
            anchors.right: parent.right
            anchors.top: label1.bottom
            anchors.leftMargin: 20
            anchors.topMargin: 10
            anchors.rightMargin: 0
        }

        ProgressBar {
            id: progressBar
            height: 20
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: bFirmwareStart.bottom
            to: 100
            anchors.leftMargin: 0
            anchors.rightMargin: 0
            anchors.topMargin: 20
            value: 0
        }

        Rectangle {
            id: rectangle
            color: "#ffffff"
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: progressBar.bottom
            anchors.bottom: parent.bottom
            anchors.rightMargin: 0
            anchors.leftMargin: 0
            anchors.bottomMargin: 0
            anchors.topMargin: 6

            ListView {
                id: listView
                anchors.fill: parent
                clip: true

                delegate: Item {
                    x: 5
                    width: listView.width - 5
                    height: textItem.contentHeight
                    Text {
                        id: textItem
                        anchors.fill: parent
                        clip: true
                        text: name
                        wrapMode: Text.WordWrap
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
                MouseArea {

                    id: mouseListView
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    anchors.rightMargin: 0
                    anchors.leftMargin: 0
                    anchors.bottomMargin: 0
                    anchors.topMargin: 0
                }

            }
        }
    }
}
