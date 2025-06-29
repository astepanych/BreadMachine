import QtQuick 2.12
//import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.12
import QtQml 2.12
import QtQuick.Window 2.15
import Qt.labs.qmlmodels 1.0
import Qt.labs.platform 1.1 as Labs


ProgramsForm {


    bReadFromFile.onClicked: {
        fileDialog.fileMode = Labs.FileDialog.OpenFile
        fileDialog.open()
    }

    bSaveToFile.onClicked: {
        fileDialog.fileMode = Labs.FileDialog.SaveFile
        fileDialog.open()
    }

    Labs.FileDialog {
        id: fileDialog

        title: "Please choose a file"
        folder: shortcuts.home
        nameFilters: "*.xml"
        onAccepted: {
            close();
            if(fileDialog.fileMode === Labs.FileDialog.SaveFile) {
                appCore.saveProgrammsToFile(fileDialog.file);
            } else if(fileDialog.fileMode === Labs.FileDialog.OpenFile) {
                appCore.readProgrammsFromFile(fileDialog.file);
            }

        }
        onRejected: {

            close();
        }

    }

    property bool isRemoveItem : false;
    bReadPrograms.onClicked:{
        appCore.readProgramms();
    }
    bWriteProgramms.onClicked: {
        appCore.writeProgramms();
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
        font.pointSize:  10
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
                contextMenu1.popup(maListProgramms);
            }
            mouse.accepted = false
        }

        onClicked: {

            if(itemDelegate.isEdit === false) {
                console.log("maListProgramms onClicked")
                itemDelegate.m_selectedRow = -1;
                itemDelegate.isSelected = false;
                if(mouse.button === Qt.RightButton) {
                    contextMenu1.popup(maListProgramms);
                }
            }
            mouse.accepted = false
        }
    }



    ///--------------------------///////////////////////////////////////////////////////////////////////

    property bool isSelected: false;
    property int m_selectedRow: -1;
    property bool isClicked: false;

    TextMetrics {
        id: headerMetrics
        font.pointSize:  15
    }

    horizontalHeader {
        delegate: Rectangle {

            id: rectangle12
            implicitHeight: horizontalHeader.height
            border.color: "lightgray"
            Text {
                text: model.display
                font.pointSize:   headerMetrics.font.pointSize

                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    listStages {

        model: paramsWorkMode
        delegate: listDelegats
        property var columnWidths: [100,150,100,100,100]

        columnWidthProvider: function(column) {
            console.log(" columnWidthProvider "+0)
            // Получаем текст заголовка
            var headerText = ""
            headerText = paramsWorkMode.horizontalHeader(column)
            console.log(" columnWidthProvider "+ headerText)
            //headerMetrics.font = horizontalHeader.font
            // Измеряем ширину текста заголовка
            headerMetrics.text = headerText
            var headerWidth = headerMetrics.advanceWidth + ((column === 4) ? 20 : 5) // + отступы
            // Возвращаем максимальную из двух ширин
            return headerWidth
        }

    }
    DelegateChooser {
        id: listDelegats
        role: "typeDelegate"
        DelegateChoice { roleValue: "textinput"; delegate: itemDelegate1}
        DelegateChoice { roleValue: "checked"; delegate: itemDelegate2}
    }
    Menu {
        id: contextMenu2
        function open(isAdd, x, y) {
            itemAddMenu.enabled = isAdd
            popup(x, y)
        }
        font.pointSize:  10

        MenuItem {
            id: itemAddMenu
            text: "Добавить этап"
            enabled: isEnableAdd//paramsWorkMode.rowCount() < 7 ? true :false
            onTriggered: {
                appCore.addStage()
            }
        }
        MenuSeparator{}
        MenuItem {
            enabled: isSelected
            text: "Удалить этап"
            onTriggered: {
                appCore.removeStage(m_selectedRow)
            }
        }
    }

    maParams {

        height: listStages.height
        propagateComposedEvents: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressAndHold: {
            isSelected = false
            m_selectedRow = -1
            if(mouse.button === Qt.LeftButton) {
                contextMenu2.open(paramsWorkMode.rowCount() < 7 ? true :false, listStages.mapToItem(window.contentItem, 0, 0).x+mouse.x, listStages.mapToItem(window.contentItem, 0, 0).y+mouse.y)
            }
            mouse.accepted = false
        }
        onClicked: {
            mouse.accepted = false

            //if(isClicked === false)
            {
                isSelected = false
                m_selectedRow = -1

                if(mouse.button === Qt.RightButton) {
                    contextMenu2.open(paramsWorkMode.rowCount() < 7 ? true :false, listStages.mapToItem(window.contentItem, 0, 0).x+mouse.x, listStages.mapToItem(window.contentItem, 0, 0).y+mouse.y)
                }
            }
            isClicked = false
        }

    }



    Component {
        id: itemDelegate1
        Item {
            id: mainItem
            z:1
            property var maxSpinsValue: [6000,300,20000]
            Rectangle {
                border.color: "lightgray"
                anchors.fill: parent
                color: (isSelected && m_selectedRow === row)? "skyblue": "#ffffff"
                MouseArea {

                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    propagateComposedEvents: true
                    onPressAndHold: {
                        var globalPos = window.contentItem.mapToGlobal(0, 0)
                        if(m_selectedRow !== row){
                            m_selectedRow = row
                            isSelected = true
                            mouse.accepted = false
                        }

                        if(mouse.button === Qt.LeftButton) {
                            contextMenu2.open(paramsWorkMode.rowCount() < 7 ? true :false, mainItem.mapToItem(window.contentItem, 0, 0).x,mainItem.mapToItem(window.contentItem, 0, 0).y)
                            mouse.accepted = true
                        }
                    }
                    onClicked: {
                        console.log(mouse.x +" onClicked 8 " + mouse.y);
                        console.log(mainItem.mapToItem(window.contentItem, 0, 0).x +" onClicked 8 " + mainItem.mapToItem(window.contentItem, 0, 0).y);
                        isClicked = true;
                        if(m_selectedRow !== row){
                            m_selectedRow = row
                            isSelected = true
                            mouse.accepted = false
                        } else {
                            mouse.accepted = false
                        }
                        if(mouse.button === Qt.RightButton) {
                            contextMenu2.open(paramsWorkMode.rowCount() < 7 ? true :false, mainItem.mapToItem(window.contentItem, 0, 0).x+mouse.x,mainItem.mapToItem(window.contentItem, 0, 0).y+mouse.y)
                            mouse.accepted = false
                        }
                    }
                    onDoubleClicked: {
                        console.log("onDoubleClicked")
                        if(mouse.button === Qt.LeftButton) {

                            var pos = mainItem.mapToItem(window.contentItem, 0, 0)
                            dialogEditor.x = window.width / 2 - dialogEditor.width/2 - pos.x
                            dialogEditor.y = window.height / 2 - dialogEditor.height/2 - pos.y

                            dialogEditor.openEditor(value, maxSpinsValue[column],(column === 2) ? 100:1);
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
                    font.pointSize: 15
                    visible: true
                }

            }

            Dialog {
                id: dialogEditor
                width: window.width / 2

                title: "Ввод значения"
                standardButtons: Dialog.Ok | Dialog.Cancel
                font.pointSize: 15
                // Настройки модальности
                modal: true
                dim: true  // Затемнение под диалогом
                Overlay.modal: Rectangle {  // Кастомное затемнение
                    color: "#80000000"  // Полупрозрачный черный
                }

                function openEditor(value, max, step) {

                    spEditor.to = Number(max)
                    spEditor.stepSize = Number(step)
                    spEditor.value = Number(value)
                    open()
                    console.log(y + " openEditor " + x)
                }

                SpinBox {

                    id: spEditor
                    anchors.fill: dialogEditor
                    width: dialogEditor.width - 20
                    value: m_value
                    from: 0
                    editable: true
                    to: m_maxValue
                    stepSize: m_stepValue
                    font.pointSize: 15
                    anchors.horizontalCenter: parent.horizontalCenter
                    valueFromText: function(text, locale) {
                        var  val = Number.fromLocaleString(locale, text)*1.0/stepSize
                        return Math.round(val)*stepSize;
                    }

                }

                onAccepted: {
                    model.value = spEditor.value
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
                Switch {
                    x:5
                    id: checkItem
                    anchors.fill: parent
                    anchors.leftMargin: 2
                    clip: true
                    checked:  value
                    anchors.verticalCenter: parent.verticalCenter
                    visible: true
                    onClicked: {
                        console.log("onClicked 3");
                    }
                    onCheckedChanged: {
                        console.log("onCheckedChanged");
                    }
                }
                MouseArea {

                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    propagateComposedEvents: true
                    onPressAndHold: {
                        console.log("onPressAndHold 3");
                        if(mouse.button === Qt.LeftButton) {
                            contextMenu2.open(paramsWorkMode.rowCount() < 7 ? true :false, checkItem.mapToItem(window.contentItem, 0, 0).x,checkItem.mapToItem(window.contentItem, 0, 0).y)
                        }
                    }
                    onClicked: {
                        isClicked = true;
                        if(m_selectedRow !== row){
                            m_selectedRow = row
                            isSelected = true
                            mouse.accepted = false
                        } else {
                            mouse.accepted = false
                        }
                        if(mouse.button === Qt.RightButton) {
                            contextMenu2.open(paramsWorkMode.rowCount() < 7 ? true :false, checkItem.mapToItem(window.contentItem, 0, 0).x,checkItem.mapToItem(window.contentItem, 0, 0).y)
                            mouse.accepted = true
                        } else {
                            checkItem.checked = !checkItem.checked
                            value = checkItem.checked
                        }


                    }

                }
            }
        }
    }

}
