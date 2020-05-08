import QtQuick 2.0
import QtMultimedia 5.15
import QtQuick.Window 2.3

Item {
    width: 800; height: 600;
    Text {
        text: "QML"
        horizontalAlignment: Text.AlignHCenter
        font.family: "Helvetica"
        font.pointSize: 24
        color: "red"
        anchors.fill: parent
    }

    VideoOutput {
        id: vo
        anchors.fill: parent
        objectName: "videoOutput"
    }
}

