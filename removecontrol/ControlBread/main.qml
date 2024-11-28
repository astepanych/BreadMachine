import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {

    id: window
    width: 600
    height: 800
    visible: true
    title: qsTr("Control breadmashine")

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        StartPage {
            id: startPage
        }
        Log{
            id: logPage
        }
    }
    footer: TabBar {
          id: tabBar
          currentIndex: swipeView.currentIndex

          TabButton {
              text: qsTr("Прошивка")
          }
          TabButton {
              text: qsTr("Журнал")
          }

      }

}
