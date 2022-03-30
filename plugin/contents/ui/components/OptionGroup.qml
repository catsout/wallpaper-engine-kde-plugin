import QtQuick 2.10
import QtQuick.Controls 2.10
import QtQuick.Layouts 1.10

Control {
    default property alias content: column_content.children
    property alias color: rect_back.color
    property alias border: rect_back.border
    property alias radius: rect_back.radius
    property alias header: option_header

    property alias show_background: rect_back.visible

    property alias item_spacing: column_content.spacing

    // the default value is not zero?
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    background: Rectangle {
        id: rect_back
        visible: false
    }

    contentItem: ColumnLayout {
        id: column_content
        anchors.left: parent.left
        anchors.right: parent.right

        OptionItem {
            id: option_header
            show_background: true
        }
    }
}
