import QtQuick 2.11
import QtQuick.Controls 2.4
import Qt.labs.platform 1.1 as Labs

LogForm {

   bClear.onClicked: {
       appCore.clearLog();
   }
   bSaveTxt.onClicked: {
       fileDialogSave.open();

   }
   bRequestLog.onClicked: {
       appCore.getLog();
   }
   listLog {
       clip: true
       model: modelMain
       delegate: Item {
           id: itemDelegate
           x: 5
           width: listLog.width - 5
           height: textItem.contentHeight
           Text {
               id: textItem
               anchors.fill: parent
               clip: true
               text: textModel
               wrapMode: Text.WordWrap
               anchors.verticalCenter: parent.verticalCenter
           }
       }
   }
   Labs.FileDialog {
       id: fileDialogSave
       fileMode: Labs.FileDialog.SaveFile
       title: "Please choose a file"
       folder: shortcuts.home
       nameFilters: "*.txt"
       onAccepted: {
           appCore.saveLog(fileDialogSave.file);

           close();
       }
       onRejected: {

          close();
       }

   }
}
