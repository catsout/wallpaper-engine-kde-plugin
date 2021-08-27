import QtQuick 2.5
import QtQuick.Layouts 1.2

Item {
    id: infoItem
    anchors.fill: parent
    property string info: "error"
    property string type: "unknown"
    property string wid: "unknown"
    GridLayout {
        id: configRow
        columns: 1
        rows: 1
        anchors.fill: parent
        Text {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: infoItem.width / 2
            text: "Shop id: " + infoItem.wid +
            "\nType: " + infoItem.type +
            "\nMessage: " + infoItem.info
            color: "yellow"
            wrapMode: Text.Wrap
            elide: Text.ElideRight 
            font.pointSize: 30
        }
    }
    Component.onCompleted:{
        background.nowBackend = "InfoShow";
    }

    function play(){}

    function pause(){}
    function getMouseTarget() {
    }
}
