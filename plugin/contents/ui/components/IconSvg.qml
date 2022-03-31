import QtQuick 2.6
import QtGraphicalEffects 1.12


Item {
    implicitWidth: 32
    implicitHeight: 32

    property alias source: image.source
    property alias fillMode: image.fillMode
    property alias smooth: image.smooth
    property alias color: overlay_color.color

    Image {
        id: image
        anchors.fill: parent

        sourceSize: Qt.size(width, height)
        fillMode: Image.PreserveAspectFit
        smooth: false
        visible: !overlay_color.visible
    }

    ColorOverlay {
        id: overlay_color
        anchors.fill: image

        visible: color != 'transparent'
        source: image
        cached: true
    }
}
