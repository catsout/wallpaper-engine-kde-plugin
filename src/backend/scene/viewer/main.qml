import QtQuick 2.14
import QtQuick.Controls 2.14

import scenetest 1.0

Item {
    width: 1280
    height: 720

    SceneViewer {
        id: renderer
        anchors.fill: parent
    }
}
