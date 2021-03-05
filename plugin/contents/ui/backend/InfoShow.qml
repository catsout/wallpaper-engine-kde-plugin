import QtQuick 2.5
import QtQuick.Layouts 1.13

Item {
    id: infoItem
    anchors.fill: parent
    property string info: "error"
    ColumnLayout {
        anchors.fill: parent
        Text {
                Layout.alignment: Qt.AlignCenter
                text: infoItem.info
                color: "yellow"
                font.pointSize: 40
        }
    }
    Component.onCompleted:{
        background.nowBackend = "InfoShow";
    }

    function play(){}

    function pause(){}
    function setMouseListener(){
    }  
}
