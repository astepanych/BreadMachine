import QtQuick 2.15
import QtQuick.Controls 2.15
import Qt.labs.qmlmodels 1.0
import QtPositioning 5.14

Page {
    id: page
    width: 400
    height: 600
    property alias horizontalHeader: horizontalHeader
    property alias verticalHeader1: verticalHeader1
    property alias bSaveToFile: bSaveToFile
    property alias bReadFromFile: bReadFromFile
    property alias rectangle2: rectangle2
    property alias rectangle: rectangle
    property alias maParams: maParams
    property alias maListProgramms: maListProgramms
    property alias pbProgramms: pbProgramms
    property alias bWriteProgramms: bWriteProgramms
    property alias bReadPrograms: bReadPrograms
    property alias listStages: listStages
    property alias listPrograms: listPrograms


    Rectangle {
        id: rectangle1
        width: 200
        height: 250
        color: "#ffffff"
        border.color: "#d3d3d3"
        border.width: 1
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 5
        anchors.topMargin: 5

        ListView {
            id: listPrograms
            anchors.fill: parent
            anchors.rightMargin: 0
            anchors.leftMargin: 0
            anchors.bottomMargin: 0
            anchors.topMargin: 0
            ScrollIndicator.vertical: ScrollIndicator {}

            MouseArea {
                id: maListProgramms
                anchors.fill: parent
            }
        }
    }

    Button {
        id: bReadPrograms
        text: qsTr("Прочитать")
        anchors.left: rectangle1.right
        anchors.right: parent.right
        anchors.top: rectangle1.top
        font.pointSize: 15
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        anchors.topMargin: 0
    }

    Button {
        id: bWriteProgramms
        height: 40
        text: qsTr("Записать")
        anchors.left: rectangle1.right
        anchors.right: parent.right
        anchors.top: bReadPrograms.bottom
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        font.pointSize: 15
        anchors.topMargin: 5
    }

    ProgressBar {
        id: pbProgramms
        anchors.left: rectangle1.right
        anchors.right: parent.right
        anchors.top: bWriteProgramms.bottom
        to: 100
        anchors.rightMargin: 5
        anchors.leftMargin: 5
        anchors.topMargin: 5
        value: appCore.percentProgressbar
    }
    Rectangle {
        id: rectangle
        color: "#ffffff"
        border.color: "#d3d3d3"
        border.width: 1
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: rectangle1.bottom
        anchors.bottom: parent.bottom
        z: 0
        anchors.bottomMargin: 5
        anchors.topMargin: 5
        anchors.rightMargin: 5
        anchors.leftMargin: 5
        VerticalHeaderView {

            id: verticalHeader1
            width: 30
            anchors.top: rectangle2.top
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            anchors.leftMargin: 0
            anchors.topMargin: 0
            anchors.left: parent.left
            syncView: listStages

            clip: true
            delegate: Rectangle {

                id: rectangle13
                implicitHeight: verticalHeader1.height
                implicitWidth: verticalHeader1.width
                border.color: "lightgray"
                Text {
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    anchors.fill: parent
                    text: model.display
                    font.pointSize: 15
                }
            }
        }
        HorizontalHeaderView {
            id: horizontalHeader
            height: 40
            anchors.left: rectangle2.left
            syncView: listStages
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 0
            anchors.topMargin: 0
            anchors.rightMargin: 0
            clip: true


        }

        Rectangle {
            id: rectangle2
            color: "#ffffff"
            anchors.left: verticalHeader1.right
            anchors.right: parent.right
            anchors.top: horizontalHeader.bottom
            anchors.bottom: parent.bottom
            anchors.leftMargin: -1
            anchors.rightMargin: 1
            anchors.bottomMargin: 1
            anchors.topMargin: -1

            TableView {
                id: listStages
                anchors.left: verticalHeader1.right
                anchors.top: horizontalHeader.bottom
                anchors.fill: parent
                z: 0

                ScrollIndicator.vertical: ScrollIndicator {}
                ScrollIndicator.horizontal: ScrollIndicator {}
                rowSpacing: -1
                columnSpacing: -1
                clip: true
            }
            MouseArea {
                id: maParams
                anchors.fill: parent

            }


        }
    }

    Button {
        id: bReadFromFile
        text: qsTr("Открыть файл")
        anchors.left: rectangle1.right
        anchors.right: parent.right
        anchors.top: pbProgramms.bottom
        font.pointSize: 15
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        anchors.topMargin: 5
    }

    Button {
        id: bSaveToFile
        text: qsTr("Сохранить файл")
        anchors.left: bReadFromFile.left
        anchors.right: bReadFromFile.right
        anchors.top: bReadFromFile.bottom
        font.pointSize: 15
        anchors.leftMargin: 0
        anchors.rightMargin: 0
        anchors.topMargin: 5
    }
}
