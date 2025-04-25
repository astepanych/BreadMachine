import QtQuick 2.15

import QtQuick.Dialogs 1.0
import Qt.labs.platform 1.1 as Labs
import com.ics.demo 1.0
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

StartPageForm {
    MyDialog{
        id:dialogError1
        x : window.width / 2 - width/2
        y : window.height / 2 - height /2
    }

    property string textDialog;


    property int dpi: Screen.pixelDensity * 25.4

        function dp(x){
            if(dpi < 120) {
                return x; // Для обычного монитора компьютера
            } else {
                return x*(dpi/160);
            }
        }

    groupBox.height: dp(groupBox.height)

    bConnect {
        enabled: !appCore.stateConnectToHost
        onClicked: {
            appCore.connectToServer(sbPort.value);
        }
    }
    bDisconnect{
        enabled: appCore.stateConnectToHost
        onClicked: {
            appCore.disconnectFromHost();
        }
    }
    mouseListView.onClicked: {
        console.log("mouse "+mouse.button);
            if(mouse.button === Qt.RightButton) {
                contextMenu.popup();
            }
        }
    bFirmware.onClicked: {
        fileDialog.open();
    }

    bFirmwareStart.onClicked: {
        appCore.startFirmware(label1.text);
    }

    Connections {
        target: appCore
        function onShowDialog(text){

            dialogError1.open(text);

        }
        function onAddStringLog(textLog){
              addImemToModel(textLog);
          }
        function onProgressFirmware(value) {
             progressBar.value = value;

        }

    }

    Labs.FileDialog {
        id: fileDialog

        title: "Please choose a file"
        folder: shortcuts.home
        nameFilters: "*.bin"
        onAccepted: {
            label1.text = fileDialog.file;
            addImemToModel("Выбран файл " + label1.text);
            close();
        }
        onRejected: {
           label1.text = "";
           close();
        }

    }

    listView.model: modelLog


    Menu {
        id: contextMenu
        MenuItem {
            text: "Clear"
            onTriggered: {
                modelLog.clear();
            }
        }
    }

    ListModel {
        id:modelLog
        onCountChanged: {
            listView.currentIndex = count - 1;
        }
    }

    function addImemToModel(str){
        modelLog.append({"name": new Date().toLocaleString(Qt.locale(),"hh-mm-ss.zzz: ") + str});
    }

   /* Dialog {
        id: dialogError
        x: window.width / 2 - width / 2
        y: window.height / 2 - height / 2
        title: "Imitator"
        standardButtons: Dialog.Ok
        Column {
            anchors.fill: parent
            Text {
                text: textDialog
            }
        }
        onAccepted: {
            //pageConnect.addImemToModel("Ошибка подключения к серверу.");
        }
    }*/

}
