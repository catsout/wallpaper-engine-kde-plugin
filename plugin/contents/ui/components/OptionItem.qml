import QtQuick 2.6
import QtQuick.Controls 2.6
import QtQuick.Layouts 1.10

Control {
    id: item_root
    property alias icon: iconsvg.source
    property alias text: text_label.text
    property alias text_color: text_label.color

    property alias color: rect_back.color
    property alias border: rect_back.border
    property alias radius: rect_back.radius


    property alias show_background: rect_back.visible

    property alias actor: control_action.contentItem
    property alias contentBottom: control_bottom.contentItem

    property alias base_height: item_option_line.height

    readonly property bool _has_bottom: contentBottom != bottom_empty

    Layout.fillWidth: true

    // default 0 to resize text
    Layout.minimumWidth: 0 //implicitWidth
    Layout.minimumHeight: implicitHeight

    leftPadding: 16
    rightPadding: 16

    // the default value is not zero?
    topPadding: 0
    bottomPadding: 0

    background: Rectangle {
        id: rect_back
        visible: false
    }
    contentItem: Item {
        id: item_content
        implicitWidth: iconsvg.width +
            (text_label.text || _has_bottom ? text_label.anchors.leftMargin + text_label.anchors.rightMargin : 0) +
            Math.max(
                text_label.implicitWidth + control_action.implicitWidth,
                (control_bottom.content_visible ? control_bottom.implicitWidth : 0)
            )
        implicitHeight: item_option_line.implicitHeight + (control_bottom.content_visible ? control_bottom.implicitHeight : 0)

        Item {
            id: item_option_line
            implicitHeight: Math.max(48, control_action.implicitHeight)
            anchors.left: parent.left
            anchors.right: parent.right

            IconSvg {
                id: iconsvg
                width: 24
                height: 24
                color: item_root.text_color
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
            }


            Text {
                id: text_label
                anchors.left: iconsvg.right
                anchors.leftMargin: 24
                anchors.right: control_action.left
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter

                textFormat: Text.PlainText
                font.bold: false
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
            }

            Control {
                id: control_action
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter

                contentItem: Item { id: action_empty }
            }
        }

        Item {
            id: _text_label_x
            x: text_label.x
        }

        Control {
            id: control_bottom
            anchors.top: item_option_line.bottom
            anchors.left: _text_label_x.left
            anchors.right: parent.right

            topPadding: 0
            leftPadding: 0
            rightPadding: 0
            bottomPadding: 0
            opacity: content_visible ? 1 : 0

            property bool content_visible: contentItem.visible

            contentItem: Item { id: bottom_empty }
        }
    }
}
