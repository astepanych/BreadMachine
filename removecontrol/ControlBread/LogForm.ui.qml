import QtQuick 2.15
import QtQuick.Controls 2.4

Page {
    id: pageLog
    width: 400
    height: 700
    property alias listLog: listLog
    property alias bSaveTxt: bSaveTxt
    property alias bClear: bClear
    property alias bRequestLog: bRequestLog

    Button {
        id: bSaveTxt
        y: 601
        text: qsTr("Сохранить в txt")
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
    }

    Button {
        id: bClear
        y: 537
        text: qsTr("Очистить")
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: bSaveTxt.top
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
    }

    Button {
        id: bRequestLog
        y: 485
        text: qsTr("Считать Журнал")
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: bClear.top
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
    }

    Rectangle {
        id: rectangle
        color: "#ffffff"
        border.width: 2
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: bRequestLog.top
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
        anchors.topMargin: 10

        ListView {
            id: listLog
            anchors.fill: parent
            anchors.rightMargin: 1
            anchors.leftMargin: 1
            anchors.bottomMargin: 1
            anchors.topMargin: 1


        }
    }
}
