import QtQuick 2.12
import "../style" as QQCC
import QtQuick.Controls 2.12
import QtQuick.Templates 2.12 as T
import QtQuick.Layouts 1.10

T.Control {
    id: item_root
    property alias icon: iconsvg.source
    property alias icon_color: iconsvg.color
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
    // Layout.minimumWidth: implicitWidth
    // Layout.minimumHeight: implicitHeight

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)


    leftPadding: 16
    rightPadding: 16


    background: Rectangle {
        id: rect_back
        visible: false
    }
    contentItem: ColumnLayout {
        id: item_content

        RowLayout {
            id: item_option_line

            IconSvg {
                id: iconsvg
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                color: item_root.text_color
            }


            Text {
                id: text_label
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 16

                textFormat: Text.PlainText
                font.bold: false
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
            }

            QQCC.Control {
                id: control_action
                Layout.preferredHeight: Math.max(48, implicitHeight)

                Layout.fillWidth: true
                Layout.maximumWidth: implicitWidth

                topPadding: 5
                bottomPadding: 5

                contentItem: Item { id: action_empty }
            }
        }
        
        QQCC.Control {
            id: control_bottom
            Layout.leftMargin: text_label.x
            Layout.fillWidth: true

            opacity: content_visible ? 1 : 0

            property bool content_visible: contentItem.visible

            contentItem: Item { id: bottom_empty }
        }
    }
}
