import QtQuick 2.5
import QtQuick.Controls 2.2

import scenetest 1.0

Item {
    id: root
    width: 1280
    height: 720

    SceneViewer {
        id: renderer
        anchors.fill: parent
        fps: 15
    }
    Timer {
        running: false
        repeat: false
        interval: 1000*5
        onTriggered: {
        }
    } 
}
