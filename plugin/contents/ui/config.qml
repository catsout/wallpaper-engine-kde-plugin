import QtQuick 2.6
import QtQuick.Controls 2.3
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.0
import QtQuick.Layouts 1.0
import org.kde.plasma.core 2.0 as PlasmaCore
// for kcm gridview
import org.kde.kcm 1.1 as KCM
import org.kde.kirigami 2.12 as Kirigami

import Qt.labs.folderlistmodel 2.12

Column {
    id: root
    anchors.fill: parent
    anchors.verticalCenter: parent.verticalCenter
    spacing: 5
    
    property string cfg_SteamLibraryPath
    property string cfg_WallpaperWorkShopId
    property string cfg_WallpaperFilePath
    property string cfg_WallpaperType
    property string cfg_BackgroundColor: "black"
    property int  cfg_DisplayMode
    property int  cfg_PauseMode
    property alias cfg_FilterMode: comboxFilter.currentIndex

    property alias cfg_MuteAudio: muteAudio.checked
    property alias cfg_CapMouse: capMouse.checked
    property alias cfg_UseMpv: useMpv.checked

    property alias cfg_Fps: sliderFps.value
    Column { 
        id: warnRow
        anchors.horizontalCenter: parent.horizontalCenter
        Text {
            text: "No qt-labs-folderlistmodel found, please install it in your linux distro."
            color: "red"
            visible: !Common.checklib_folderlist(warnRow) 
        }
        Text {
            text: "Scene wallpaper may crash kde earily, make sure you know how to fix."
            color: "yellow"
            visible: Common.checklib_wallpaper(warnRow)
        }
    }
    Row {
        id: configRow
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 30
        Column {
            id: configCol
            anchors.verticalCenter: parent.verticalCenter
            Row {
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Pause:"
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
            }
            Row {
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Display:"
                }
                ComboBox {
                    id: displayMode
                    model: [
                        {
                            text: "Keep aspect radio",
                            value: Common.DisplayMode.Aspect
                        },
                        {
                            text: "Scaling and crop",
                            value: Common.DisplayMode.Crop
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
            CheckBox {
                id: muteAudio
                text: "Mute Audio"
            }          
            CheckBox {
                id: capMouse
                text: "Capture Mouse"
                hoverEnabled: true

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Use ctrl+shift+z to switch")

            }
            Row {
                visible: Common.checklib_wallpaper(configCol)
                CheckBox{
                    id: useMpv
                    text: "Use mpv"
                }
            }
            Row {
                spacing: fpsLabel.width * 0.2
                Label{
                    id: fpsLabel
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Fps: " + (sliderFps.value<10?"0":"") + sliderFps.value.toString()
                }
                Slider {
                    id: sliderFps
                    anchors.verticalCenter: parent.verticalCenter
                    width: configCol.width - fpsLabel.width * 1.4
                    from: 5
                    to: 60
                    stepSize: 1.0
                    snapMode: Slider.SnapOnRelease
                }
            }
        }
        AnimatedImage {
            id: previewAnim
            anchors.verticalCenter: parent.verticalCenter

            property var nullItem: QtObject {
                property string source: ""
            }
            property var picItem: picView.view.currentItem ? picView.view.currentItem.thumbnail[1] : nullItem

            height: configCol.height
            width: height*(16.0/9.0)
            source: picItem.source
            onStatusChanged: playing = (status == AnimatedImage.Ready) 
            onPicItemChanged: {
                if(picItem != nullItem) {
                    height = picItem.height * 1.5;
                }
            }
        }
    }

    Row {
        id: infoRow
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 10

        Label {
            id: workshopidLabel
            anchors.verticalCenter: parent.verticalCenter
            text: 'Shopid: <a href="https://steamcommunity.com/sharedfiles/filedetails/?id='+ cfg_WallpaperWorkShopId + '">'+ cfg_WallpaperWorkShopId +'</a>'
            onLinkActivated: Qt.openUrlExternally(link)
        }
        Label {
            anchors.verticalCenter: parent.verticalCenter
            text: "Type: " + cfg_WallpaperType
        }
        Button {
            id: wpFolderButton
            anchors.verticalCenter: parent.verticalCenter
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
            anchors.verticalCenter: parent.verticalCenter
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
        ComboBox {
            id: comboxFilter
            anchors.verticalCenter: parent.verticalCenter
            model: [
                {
                    text: "Show All",
                    value: "All"
                },
                {
                    text: "Show Scene",
                    value: "scene"
                },
                {
                    text: "Show Web",
                    value: "web"
                },
                {
                    text: "Show Video",
                    value: "video"
                }
            ]
            textRole: "text"
            valueRole: "value"
            onActivated: {
                wplist.upToListModel(getFilter());
            }
            function getFilter() {
                return function(el) {
                    if(currentValue === "All" || el.type === currentValue) 
                        return true;
                    else
                        return false;
                };
            }
        }
    }


    FolderListModel {
        id: wplist
        // use var not list as doc
        property var files
        property bool lock: false
        folder: cfg_SteamLibraryPath + Common.wpenginePath
        onStatusChanged: {
            if (wplist.status == FolderListModel.Ready && cfg_SteamLibraryPath !== "")
            {
                wplist.files = [];
                wplist.lock = false;
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
                    Common.readTextFile(el.path + "/project.json", (text) => readCallback(text, el));
                });
            }
        }
        function readCallback(text, el) {
            let project = Common.parseJson(text);    
            if(project !== null) {
                if("title" in project)
                    el.title = project.title;
                if("preview" in project)
                    el.preview = project.preview;
                if("file" in project)
                    el.file = project.file;
                if("type" in project)
                    el.type = project.type.toLowerCase();
            }
            el.loaded = true;
            check();
        }
        function check() {
            for(let i=0;i < wplist.files.length;i++)
                if(!wplist.files[i].loaded) return;
            if(wplist.lock) return;
            wplist.lock = true; 
            upToListModel(comboxFilter.getFilter());
        }
        function upToListModel(filterFunc) {
            if(typeof(filterFunc) === "undefined" )
                filterFunc = (el) => true;

            projectModel.clear();
            // 0 for default
            let currentIndex = wplist.files.length==0?-1:0;
            wplist.files.forEach(function(el) {
                if(!filterFunc(el)) return;
                projectModel.append(el);
                if(el.workshopid === cfg_WallpaperWorkShopId)
                    currentIndex = projectModel.count - 1;
            });
            picView.view.currentIndex = currentIndex;

        }
    }


    ListModel {
        id: projectModel
    }
    KCM.GridView {
        id: picView
        height: root.height - configRow.height - infoRow.height - warnRow.height
        anchors.left: parent.left
        anchors.right: parent.right

        view.model: projectModel 
        view.delegate: KCM.GridDelegate {
            text: title
            actions: [
            Kirigami.Action {
                icon.name: "document-open-folder"
                tooltip: "Open Containing Folder"
                onTriggered: Qt.openUrlExternally(path) 
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
                   // picView.view.currentItem.thumbnail[1]
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
