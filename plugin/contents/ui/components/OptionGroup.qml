import QtQuick 2.12
import QtQuick.Templates 2.12 as T
import QtQuick.Layouts 1.10

T.Control {
    default property alias content: column_content.children
    property alias color: rect_back.color
    property alias border: rect_back.border
    property alias radius: rect_back.radius
    property alias header: option_header

    property alias show_background: rect_back.visible

    property alias item_spacing: column_content.spacing

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    background: Rectangle {
        id: rect_back
        visible: false
    }

    contentItem: ColumnLayout {
        id: column_content
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 3

        OptionItem {
            id: option_header
            show_background: true
        }
    }
}
