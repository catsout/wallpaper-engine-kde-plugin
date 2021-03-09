import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.0
import QtQuick.Layouts 1.0
import org.kde.plasma.core 2.0 as PlasmaCore
// for kcm gridview
import org.kde.kcm 1.1 as KCM
import org.kde.kirigami 2.12 as Kirigami

import Qt.labs.folderlistmodel 2.12


ColumnLayout {
    id: root
    Layout.alignment: Qt.AlignCenter
    
    property string cfg_SteamLibraryPath
    property string cfg_WallpaperWorkShopId
    property string cfg_WallpaperFilePath
    property string cfg_WallpaperType
    property string cfg_BackgroundColor: "black"
    property int  cfg_DisplayMode
    property int  cfg_PauseMode

    property alias cfg_MuteAudio: muteAudio.checked
    property alias cfg_CapMouse: capMouse.checked
    property alias cfg_UseMpv: useMpv.checked

    RowLayout {
        id: selectRow
        Layout.alignment: Qt.AlignCenter
        Layout.topMargin: 10.0

        Label {
            text: "Pause:"
            Layout.alignment: Qt.AlignLeft 
        }
        
        ComboBox {
            id: pauseMode
            model: [
                {
                    text: "Maximied Window",
                    value: Common.PauseMode.Max
                },
                {
                    text: "Any Window",
                    value: Common.PauseMode.Any
                },
                {
                    text: "Never Pause",
                    value: Common.PauseMode.Never
                }
            ]
            textRole: "text"
            valueRole: "value"
            onActivated: cfg_PauseMode = currentValue
            Component.onCompleted: currentIndex = indexOfValue(cfg_PauseMode)
        }

        Label {
            text: "|"
            Layout.alignment: Qt.AlignLeft 
        }
 

        Label {
            text: "Display:"
            Layout.alignment: Qt.AlignLeft 
        }
        
        ComboBox {
            id: displayMode
            model: [
                {
                    text: "Keep aspect radio",
                    value: Common.DisplayMode.Aspect
                },
                {
                    text: "Scale to fill",
                    value: Common.DisplayMode.Scale
                },
            ]
            textRole: "text"
            valueRole: "value"
            onActivated: cfg_DisplayMode = currentValue
            Component.onCompleted: currentIndex = indexOfValue(cfg_DisplayMode)
        }
        
    }
    Text {
        Layout.alignment: Qt.AlignCenter
        text: "No qt-labs-folderlistmodel found, please install it in your linux distro."
        color: "red"
        visible: !Common.checklib_folderlist(checkRow) 
    }
    Text {
        Layout.alignment: Qt.AlignCenter
        text: "Scene wallpaper may crash kde earily, make sure you know how to fix."
        color: "yellow"
        visible: Common.checklib_wallpaper(checkRow)
    }
    RowLayout {
        id: checkRow
        Layout.alignment: Qt.AlignCenter
        CheckBox {
            id: muteAudio
            text: "Mute Audio"
        }          
        Label{
            text: "|"
            Layout.alignment: Qt.AlignLeft 
        }
        CheckBox {
            id: capMouse
            text: "Capture Mouse"
            hoverEnabled: true

            ToolTip.visible: hovered
            ToolTip.text: qsTr("Use ctrl+shift+z to switch")

        }
        RowLayout{
            visible: Common.checklib_wallpaper(checkRow)
            Label{
                text: "|"
                Layout.alignment: Qt.AlignLeft 
            }
            CheckBox{
                id: useMpv
                text: "Use mpv"
            }
        }

    }

    RowLayout {
        id: infoRow
        Layout.alignment: Qt.AlignCenter
        

        Label {
            id: workshopidLabel
            text: 'Shopid: <a href="https://steamcommunity.com/sharedfiles/filedetails/?id='+ cfg_WallpaperWorkShopId + '">'+ cfg_WallpaperWorkShopId +'</a>'
            onLinkActivated: Qt.openUrlExternally(link)
        }
        Label {text: '|'}
        Label {
            text: "Type: " + cfg_WallpaperType
            Layout.alignment: Qt.AlignLeft
        }
        Label {text: '|'}
        Button {
            id: wpFolderButton
            implicitWidth: height
            PlasmaCore.IconItem {
                anchors.fill: parent
                source: "folder-symbolic"
                PlasmaCore.ToolTipArea {
                    anchors.fill: parent
                    subText: cfg_SteamLibraryPath?cfg_SteamLibraryPath:"Select steam libary dir"
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: { wpDialog.open() }
            }
        }
        Button {
            id: refreshButton
            implicitWidth: height
            PlasmaCore.IconItem {
                anchors.fill: parent
                source: "view-refresh"
                PlasmaCore.ToolTipArea {
                    anchors.fill: parent
                    subText: ""
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: { 
                    projectModel.clear();
                    var url = wplist.folder;
                    wplist.folder = "";
                    wplist.folder = url;
                }
            }
        }
    }

    FolderListModel {
        id: wplist
        // use var not list as doc
        property var files
        property var name_to_index
        property bool lock: false
        folder: cfg_SteamLibraryPath + Common.wpenginePath//"/steamapps/workshop/content/431960" 
        onStatusChanged: {
            if (wplist.status == FolderListModel.Ready && cfg_SteamLibraryPath !== "")
            {
                wplist.files = [];
                wplist.name_to_index = {};
                wplist.lock = false;
                projectModel.clear();
                for(let i=0;i < wplist.count;i++)
                {
                    let v = {
                        "workshopid": wplist.get(i,"fileName"),
                        "path": wplist.get(i,"filePath"),
                        "loaded": false,
                        "title": "unknown",
                        "preview": "unknown",
                        "type": "unknown",
                    };
                    wplist.files.push(v);
                }
                wplist.files.forEach(function(el) {
                    Common.readTextFile(el["path"] + "/project.json", (text) => readCallback(text, el));
                });
            }
        }
        function readCallback(text, el) {
            //console.log("read project:" + v["workshopid"])
            let project = Common.parseJson(text);    
            if(project !== null) {
                if("title" in project)
                    el["title"] = project["title"];
                if("preview" in project)
                    el["preview"] = project["preview"];
                if("file" in project)
                    el["file"] = project["file"];
                if("type" in project)
                    el["type"] = project["type"].toLowerCase();
            }
            el["loaded"] = true;
            check();
        }
        function check() {
            for(let i=0;i < wplist.files.length;i++)
                if(!wplist.files[i]["loaded"]) return;
            if(wplist.lock) return;
            wplist.lock = true; 
            // -1 for no select
            let currentIndex = -1;
            wplist.files.forEach(function(el, index) {
                projectModel.append(el);
                if(el["workshopid"] === cfg_WallpaperWorkShopId)
                    currentIndex = index;
            });
 //           projectModel.sync();
            picView.view.currentIndex = currentIndex;
        }
    }


    ListModel {
        id: projectModel
        function openContainingFolder(index){
            var now = projectModel.get(index)
            Qt.openUrlExternally("file://"+now["path"]) 
        }
    }

    KCM.GridView {
        id: picView
        Layout.fillWidth: true
        Layout.fillHeight: true

        view.model: projectModel 
        view.delegate: KCM.GridDelegate {
            text: title
            actions: [
            Kirigami.Action {
                icon.name: "document-open-folder"
                tooltip: "Open Containing Folder"
                onTriggered: projectModel.openContainingFolder(index)
            }]
            thumbnail:Image {
                anchors.fill: parent
                source: path + "/" + preview
            }
            onClicked: {
                   cfg_WallpaperWorkShopId = workshopid;
                   cfg_WallpaperFilePath = path + "/" + file;
                   cfg_WallpaperType = type;
                   picView.view.currentIndex = index;
               }
            }
    }

    FileDialog {
        id: wpDialog
        title: "Select steam libary dir"
        selectFolder: true
        selectMultiple : false
        nameFilters: [ "All files (*)" ]
        onAccepted: {
            var path = Common.trimCharR(wpDialog.fileUrls[0], '/');
            cfg_SteamLibraryPath = path;
        }
    }

    onCfg_WallpaperTypeChanged: {}
}
