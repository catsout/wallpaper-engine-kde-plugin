import QtQuick 2.5
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.2
import org.kde.plasma.core 2.0 as PlasmaCore
// for kcm gridview
import org.kde.kcm 1.1 as KCM
import org.kde.kirigami 2.4 as Kirigami

import Qt.labs.folderlistmodel 2.11

Column {
    id: root
    anchors.fill: parent
    anchors.verticalCenter: parent.verticalCenter
    anchors.topMargin: 5
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
    property alias cfg_UseMpv: useMpv.checked

    property alias cfg_Fps: sliderFps.value
    Column { 
        id: warnRow
        anchors.horizontalCenter: parent.horizontalCenter
        Text {
            text: "Scene wallpaper may crash kde earily, make sure you know how to fix."
            color: "darkorange"
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
                    onActivated: cfg_PauseMode = Common.cbCurrentValue(pauseMode)
                    Component.onCompleted: currentIndex = Common.cbIndexOfValue(pauseMode, cfg_PauseMode)
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
                    onActivated: cfg_DisplayMode = Common.cbCurrentValue(displayMode)
                    Component.onCompleted: currentIndex = Common.cbIndexOfValue(displayMode, cfg_DisplayMode)
                }
            }
            CheckBox {
                id: muteAudio
                text: "Mute Audio"
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
                    ToolTip.visible: fpsMouse.containsMouse
                    ToolTip.text: qsTr("Control fps on scene wallpaper")
                    MouseArea {
                        id: fpsMouse
                        anchors.fill: parent
                        hoverEnabled: true
                    }
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
            property var picItem: (picViewLoader.status == Loader.Ready && picViewLoader.item.view.currentItem)
                                ? picViewLoader.item.view.currentItem
                                : nullItem
            height: configCol.height
            width: height*(16.0/9.0)
            source: ""
            cache: false
            asynchronous: true
            onStatusChanged: playing = (status == AnimatedImage.Ready) 
            onPicItemChanged: {
                if(picItem != nullItem) {
                    height = picItem.thumbnail[1].height * 1.5;
                    source = picItem.thumbnail[1].source;
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
            onActivated: {
                folderWorker.filter();
            }
        }
    }

    WorkerScript {
        id: folderWorker
        source: "folderWorker.mjs"
        // use var not list as doc
        property var proxyModel
        onMessage: {
            if(messageObject.reply == "loadFolder") {
                proxyModel = messageObject.data;
                filter();
            } else if(messageObject.reply == "filter") {
                if(picViewLoader.status == Loader.Ready) {
                    picViewLoader.item.setCurIndex(); 
                }
            }
        }
        function filter() {
            let msg = {
                "action": "filter", 
                "data": folderWorker.proxyModel,
                "model": projectModel,
                "type": Common.cbCurrentValue(comboxFilter)
            };
            folderWorker.sendMessage(msg);
        }
    }

    FolderListModel {
        id: wplist
        folder: cfg_SteamLibraryPath + Common.wpenginePath
        onStatusChanged: {
            if(cfg_SteamLibraryPath === "")
                return;
            if (wplist.status == FolderListModel.Ready) {
                new Promise(function (resolve, reject) {
                    folderWorker.proxyModel = [];
                    for(let i=0;i < wplist.count;i++) {
                        let v = {
                            "workshopid": wplist.get(i,"fileName"),
                            "path": wplist.get(i,"filePath"),
                            "loaded": false,
                            "title": "unknown",
                            "preview": "unknown",
                            "type": "unknown",
                        };
                        folderWorker.proxyModel.push(v);
                    }
                    resolve();
                }).then(function(value) {    
                    let msg = {"action": "loadFolder", "data": folderWorker.proxyModel};
                    folderWorker.sendMessage(msg);
                });
            }
        }
    }

    ListModel {
        id: projectModel
    }

    Loader {
        id: picViewLoader
        asynchronous: true
        sourceComponent: picViewCom
        visible: status == Loader.Ready

        height: root.height - configRow.height - infoRow.height - warnRow.height - root.spacing*3
        anchors.left: parent.left
        anchors.right: parent.right
        onStatusChanged: {
            if(status == Loader.Ready)
                picViewLoader.item.setCurIndex();
        }
    }
    Component { 
        id: picViewCom
        KCM.GridView {
            id: picViewGrid
            anchors.fill: parent

            view.model: projectModel
            view.delegate: KCM.GridDelegate {
                text: title
                actions: [
                Kirigami.Action {
                    icon.name: "document-open-folder"
                    tooltip: "Open Containing Folder"
                    onTriggered: Qt.openUrlExternally(path) 
                }]
                thumbnail: Image {
                    anchors.fill: parent
                    source: path + "/" + preview
                    sourceSize.width: width
                    sourceSize.height: height
                    fillMode: Image.Stretch
                    cache: false
                    asynchronous: true
                }
                onClicked: {
                       cfg_WallpaperFilePath = path + "/" + file;
                       cfg_WallpaperType = type;
                       cfg_WallpaperWorkShopId = workshopid;
                       view.currentIndex = index;
                }
            }

            Kirigami.Heading {
                anchors.fill: parent
                anchors.margins: Kirigami.Units.largeSpacing
                // FIXME: this is needed to vertically center it in the grid for some reason
                anchors.topMargin: picViewGrid.height
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
                visible: picViewGrid.view.count === 0
                level: 2
                text: cfg_SteamLibraryPath
                    ?"There are no wallpapers in steam libary workshop"
                    :"Select your steam libary through the folder selector above"
                opacity: 0.5
            }

            function setCurIndex() {
                new Promise(function(reoslve, reject) {
                    for(let i=0;i < projectModel.count;i++) {
                        if(projectModel.get(i).workshopid === cfg_WallpaperWorkShopId) {
                            view.currentIndex = i;
                            break;
                        }
                    }
                    if(view.currentIndex == -1 && projectModel.count != 0)
                        view.currentIndex = 0;

                    if(!cfg_WallpaperFilePath || cfg_WallpaperFilePath == "")
                        if(view.currentIndex != -1)
                            view.currentItem.onClicked();

                    resolve();
                });
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
