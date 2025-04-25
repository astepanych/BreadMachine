import QtQuick 2.4
import QtQuick.Controls 2.15
import Qt.labs.qmlmodels 1.0

ProgramsForm {

    property bool isRemoveItem : false;
    bReadPrograms.onClicked:{
        appCore.readProgramms();
    }

    listPrograms {
        clip: true
        model: modelPrograms
        delegate: itemDelegate.programmNameDelegate
        onCurrentIndexChanged: {
            console.log("1 cur = ");
        }
    }
    ProrammsNameDelegate {
        id: itemDelegate
        m_model: modelPrograms
        onSwitchItem: {
            appCore.switchModelPrograms(index);
        }
    }

    Connections {
        target: appCore
        function onAddedProgramms() {
            if (listPrograms.count > 0) {
                listPrograms.positionViewAtIndex(listPrograms.count - 1, ListView.End);
                itemDelegate.m_selectedRow = listPrograms.count - 1
            }
        }
    }

    Menu {
        id: contextMenu1
        MenuItem {
            text: "Добавить программу"
            enabled: true
            onTriggered: {
                appCore.addPrograms();
            }
        }
        MenuSeparator{}
        MenuItem {
            enabled: itemDelegate.isSelected
            text: "Удалить программу"
            onTriggered: {
                isRemoveItem = true
                appCore.removePrograms(itemDelegate.m_selectedRow)
            }
        }
    }
    maListProgramms {
        propagateComposedEvents: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressAndHold: {
            console.log("1 onPressAndHold")
            if(mouse.button === Qt.LeftButton) {
                contextMenu1.popup();
            }
            mouse.accepted = false
        }

        onClicked: {
            if(itemDelegate.isEdit === false) {
                console.log("maListProgramms onClicked")
                itemDelegate.m_selectedRow = -1;
                itemDelegate.isSelected = false;
                if(mouse.button === Qt.RightButton) {
                    contextMenu1.popup();
                }
            }
            mouse.accepted = false
        }
    }



    ///--------------------------///////////////////////////////////////////////////////////////////////

    property bool isSelected: false;
    property int m_selectedRow: -1;
    property bool isClicked: false;
    listStages {
        model: paramsWorkMode
        delegate: listDelegats
        property var columnWidths: [100,150,100,100,100]
        columnWidthProvider: function (column) { return columnWidths[column] }

    }
    DelegateChooser {
        id: listDelegats
        role: "typeDelegate"
        DelegateChoice { roleValue: "textinput"; delegate: itemDelegate1}
        DelegateChoice { roleValue: "checked"; delegate: itemDelegate2}
    }
    Menu {
        id: contextMenu2
        MenuItem {
            text: "Добавить этап"
            enabled: true
            onTriggered: {
                appCore.addStage()
            }
        }
        MenuSeparator{}
        MenuItem {
           // enabled: itemDelegate.isSelected
            text: "Удалить этап"
            onTriggered: {
                appCore.removeStage(m_selectedRow)

            }
        }
    }

    maParams {

        //height: listStages.height
        propagateComposedEvents: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressAndHold: {
            console.log("onPressAndHold" + 1);

            if(mouse.button === Qt.LeftButton) {
                contextMenu2.popup();
            }
            mouse.accepted = false
        }
        onClicked: {
            console.log("maParams onClicked")
            if(isClicked === false) {
                isSelected = false
                m_selectedRow = -1

                mouse.accepted = true
                if(mouse.button === Qt.RightButton) {
                    contextMenu2.popup()
                }
            }
            isClicked = false
        }

    }

    Component {
        id: itemDelegate1
        Item {
            Rectangle {
                border.color: "lightgray"
                anchors.fill: parent
                color: (isSelected && m_selectedRow === row)? "skyblue": "#ffffff"
                MouseArea {

                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    propagateComposedEvents: true
                    onPressAndHold: {
                        console.log("onPressAndHold");
                        if(mouse.button === Qt.LeftButton) {
                            contextMenu2.popup()
                        }
                    }
                    onClicked: {
                        isClicked = true;
                        if(m_selectedRow !== row){
                            m_selectedRow = row
                            isSelected = true
                            mouse.accepted = false
                        } else {
                            mouse.accepted = true
                        }
                        if(mouse.button === Qt.RightButton) {
                            contextMenu2.popup();
                            mouse.accepted = true
                        }

                        console.log("in onClicked  " + m_selectedRow)

                    }
                    onDoubleClicked: {
                        console.log("2 onDoubleClicked")
                        if(mouse.button === Qt.LeftButton) {

                        }
                    }
                }
                Text {
                    x:5
                    id: textItem
                    anchors.fill: parent
                    anchors.leftMargin: 2
                    clip: true
                    text: value
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    anchors.verticalCenter: parent.verticalCenter
                    font.pointSize: 12
                    visible: true
                }

            }
        }
    }

    Component {
        id: itemDelegate2
        Item {
            Rectangle {
                border.color: "lightgray"
                anchors.fill: parent
                color: (isSelected && m_selectedRow === row)? "skyblue": "#ffffff"
                CheckBox {
                    x:5
                    id: textItem
                    anchors.fill: parent
                    anchors.leftMargin: 2
                    clip: true
                    checked:  value
                    anchors.verticalCenter: parent.verticalCenter
                    font.pointSize: 12
                    visible: true
                }
                MouseArea {

                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    propagateComposedEvents: true
                    onPressAndHold: {
                        console.log("onPressAndHold 3");
                        if(mouse.button === Qt.LeftButton) {
                            contextMenu2.popup();
                        }
                    }
                    onClicked: {
                        isClicked = true;
                        if(m_selectedRow !== row){
                            m_selectedRow = row
                            isSelected = true
                            mouse.accepted = false
                        } else {
                            mouse.accepted = true
                        }
                        if(mouse.button === Qt.RightButton) {
                            contextMenu2.popup();
                            mouse.accepted = true
                        }
                        console.log("in onClicked  " + m_selectedRow)
                        console.log("h = " + rectangle.height+"; h1 = "+maParams.height)
                    }

                }
            }
        }
    }

}
