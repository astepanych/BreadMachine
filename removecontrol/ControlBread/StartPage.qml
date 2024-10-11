import QtQuick 2.15
import QtQuick.Controls 2.15

StartPageForm {
    property string textDialog;
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
    Connections {
           target: appCore
        function onShowDialog(text){
            textDialog = text;
            dialogError.open();
        }
    }

    Dialog {
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
       }

}
