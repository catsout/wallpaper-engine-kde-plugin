import QtQuick 2.5
import QtQuick.Controls 2.2

import scenetest 1.0

Item {
    width: 1280
    height: 720
    Loader {
        id: loader
        anchors.fill: parent
        sourceComponent: rect
    }

    Component {
        id: rect
        SceneViewer {
            id: renderer
            anchors.fill: parent
            fps: 15
        }
    }

    Timer {
        running: false
        repeat: false
        interval: 1000*5
        onTriggered: {
            loader.sourceComponent = undefined;
        }
    } 
}
