import QtQuick 2.6
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.5
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kirigami 2.4 as Kirigami

import ".."

Kirigami.FormLayout {
    Layout.fillWidth: true
    twinFormLayouts: parentLayout
    PlasmaComponents.TextArea {
        Layout.fillWidth: true
        Kirigami.FormData.label: {
            Kirigami.FormData.label = "Requirements:";
            if(Kirigami.FormData.labelAlignment !== undefined) 
                Kirigami.FormData.labelAlignment = Qt.AlignTop;
        }
        implicitWidth: 0
        text: `
            <ol>
            <li><i>Wallpaper Engine</i> installed on Steam</li>
            <li>Subscribe to some wallpapers on the Workshop</li>
            <li>Select the <i>steamlibrary</i> folder on the Wallpapers tab of this plugin
                <ul>
                    <li>The <i>steamlibrary</i> which contains the <i>steamapps<i/> folder</li>
                    <li><i>Wallpaper Engine</i> needs to be installed in this <i>steamlibrary</i></li>
                </ul>
            </li>
            </ol>
        `
        wrapMode: Text.Wrap
        textFormat: Text.RichText
        readOnly: true
        selectByMouse: false
    }
    PlasmaComponents.TextArea {
        Layout.fillWidth: true
        Kirigami.FormData.label: {
            Kirigami.FormData.label = "Fix crashes:";
            if(Kirigami.FormData.labelAlignment !== undefined)
                Kirigami.FormData.labelAlignment = Qt.AlignTop;
        }
        implicitWidth: 0
        text: `
            <ol>
            <li>Remove <i>WallpaperFilePath</i> line in <b>~/.config/plasma-org.kde.plasma.desktop-appletsrc</b></li>
            <li>Restart KDE</li>
            </ol>
        `
        readOnly: true
        wrapMode: Text.Wrap
        textFormat: Text.RichText
        selectByMouse: true
        visible: libcheck.wallpaper
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            cursorShape: Qt.IBeamCursor
        }
    }
    PlasmaComponents.Label {
        Kirigami.FormData.label: {
            Kirigami.FormData.label = "Readme:";
            if(Kirigami.FormData.labelAlignment !== undefined)
                Kirigami.FormData.labelAlignment = Qt.AlignTop;
        }
        text: '<a href="https://github.com/catsout/wallpaper-engine-kde-plugin">repo</a>'
        onLinkActivated: Qt.openUrlExternally(link)
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            cursorShape: Qt.PointingHandCursor
        }
    }
    ListView {
        Layout.fillWidth: true
        Kirigami.FormData.label: {
            Kirigami.FormData.label = "Lib check:";
            if(Kirigami.FormData.labelAlignment !== undefined)
                Kirigami.FormData.labelAlignment = Qt.AlignTop;
        }
        implicitHeight: (font.pixelSize * 2) * modelraw.length
        model: ListModel {}
        clip: false
        property var modelraw: {
            const _model = [
                {
                    ok: libcheck.folderlist,
                    name: "*qt-lab-folderlist"
                },
                {
                    ok: libcheck.qtwebsockets,
                    name: "*qtwebsockets (qml)"
                },
                {
                    ok: pyext && pyext.ok,
                    name: "*python3-websockets"
                },
                {
                    ok: libcheck.qtwebchannel,
                    name: "qtwebchannel (qml) (for web)"
                },
                {
                    ok: libcheck.wallpaper,
                    name: "plugin lib (for scene,mpv)"
                }
            ];
            return _model;
        }
        onModelrawChanged: {
            this.model.clear();
            this.modelraw.forEach((el) => {
                this.model.append(el);
            });
        }
        delegate: CheckBox {
            text: name
            checked: ok
            enabled: false
        }

    }
}
