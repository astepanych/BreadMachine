import QtQuick 2.15
import QtQuick.Controls 2.4

Page {
    id: pageConnect
    width: 400
    height: 600
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

        height: 170
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.topMargin: 10
        title: qsTr("Подключение к устройству")

        SpinBox {
            id: sbPort
            y: 168
            height: 40
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 5
            value: 33100
            to: 65535
            from: 1
            anchors.rightMargin: 5
        }

        Button {
            id: bConnect
            width: (parent.width / 2) - 20
            text: qsTr("Подключить")
            anchors.left: parent.left
            anchors.top: rIpAddr.bottom
            anchors.bottom: parent.bottom
            topPadding: 12
            anchors.bottomMargin: 10
            anchors.leftMargin: 0
            anchors.topMargin: 10
        }

        Button {
            id: bDisconnect
            text: qsTr("Отключить")
            anchors.left: bConnect.right
            anchors.right: parent.right
            anchors.top: rIpAddr.bottom
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.leftMargin: 20
            anchors.rightMargin: 0
            anchors.topMargin: 10
            enabled: false
        }

        Label {
            id: label
            y: 146
            text: qsTr("Номер порта")
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
