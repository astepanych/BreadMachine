import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.15

Item {
    property string m_textDialog;
    width: dialogError.width
    height: dialogError.height
    function open (text1) {
        console.log(text1)

        dialogError.open()
        m_textDialog = text1
    }

    Dialog {
           id: dialogError

           title: "Control"
           standardButtons: Dialog.Ok
           Column {
               anchors.fill: dialogError
               Text {
                   text: m_textDialog
               }
           }
           onAccepted: {

           }
       }
}
