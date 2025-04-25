import QtQuick 2.4
import QtQuick.Controls 2.15

Item {
    property Component programmNameDelegate: programmNameDelegate
    property int m_implicitWidth: m_implicitWidth

    property var m_model;
    property bool isSetCurrentIndexSoft : false;
    property bool isSelected : false;
    property int m_selectedRow : -1;
    property bool isEdit: false


    signal switchItem(int index);

    clip: true
    anchors.fill: parent
    Component {
        id: programmNameDelegate
        Item {
            width: listPrograms.width
            height: textItem.contentHeight+15
            Rectangle {
                border.color: "lightgray"
                anchors.fill: parent
                color: (isSelected && m_selectedRow === index)? "skyblue": "#ffffff"
                MouseArea {
                    z:1
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    propagateComposedEvents: true
                    onPressAndHold: {
                        if(mouse.button === Qt.LeftButton) {
                            contextMenu1.popup();
                        }
                    }
                    onClicked: {

                        console.log("2 onClicked")
                        if(m_selectedRow !== index){
                            m_selectedRow = index;
                            isEdit = false
                            isSelected = true;
                            switchItem(index);
                            if(mouse.button === Qt.RightButton) {
                                contextMenu1.popup();
                            }
                            mouse.accepted = false
                        } else {

                            mouse.accepted = true
                        }


                    }
                    onDoubleClicked: {
                        console.log("2 onDoubleClicked")
                        if(mouse.button === Qt.LeftButton) {
                            isSelected = false;
                            m_selectedRow = index;
                            isEdit = true

                            textInput.forceActiveFocus()
                        }
                    }
                }
                Text {
                    x:5
                    id: textItem
                    anchors.fill: parent
                    anchors.leftMargin: 2
                    clip: true
                    text: namemode
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                    anchors.verticalCenter: parent.verticalCenter
                    font.pointSize: 12
                    visible: !textInput.visible
                    onFocusChanged: {
                        console.log("focus1 = " + focus)
                    }
                }
                TextInput {
                    z:1
                    x:5
                    id: textInput
                    anchors.fill: parent
                    clip: true

                    text: namemode
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.WordWrap
                    anchors.verticalCenter: parent.verticalCenter
                    font.pointSize: 12
                    font.italic:  true
                    visible: (isEdit && m_selectedRow === index)
                    onVisibleChanged: {
                        console.log("index " + index+", visible " + visible)
                        if(visible === false && namemode !== text) {
                            namemode = text
                            //isEdit = false
                            console.log(text)
                        }
                    }

                    onFocusChanged: {
                        console.log("focus2 = " + focus)
                    }

                }
            }
        }
    }

}
