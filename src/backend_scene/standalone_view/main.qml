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
        running: true
        repeat: false
        interval: 1000*8
        onTriggered: {
            renderer.source = 'file:///var/home/out/Documents/myGit/wallpaper/src/backend_scene/standalone_view/build/431960/2807856301/scene.pkg';
        }
    } 
}
