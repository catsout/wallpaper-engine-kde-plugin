import QtQuick 2.6
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
    property string cfg_FilterStr
    property int  cfg_DisplayMode
    property int  cfg_PauseMode
    property alias cfg_Volume: sliderVol.value
    property alias cfg_FilterMode: comboxFilter.currentIndex

    property alias cfg_MuteAudio: muteAudio.checked
    property alias cfg_UseMpv: useMpv.checked

    property alias cfg_Fps: sliderFps.value
    Column { 
        id: warnRow
        anchors.horizontalCenter: parent.horizontalCenter
        Text {
            text: "Scene wallpaper may crash kde, make sure you know how to fix."
            color: "darkorange"
            visible: Common.checklib_wallpaper(warnRow)
        }
    }
    GridLayout {
        id: configRow
        columns: 2
        anchors.horizontalCenter: parent.horizontalCenter
        GridLayout {
            id: configCol
            columns: 2
            Layout.maximumWidth: 300
            Label {
                text: "Pause"
            }
            ComboBox {
                id: pauseMode
                model: [
                    {
                        text: "Maximized Window",
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
            Label {
                text: "Display"
            }
            ComboBox {
                id: displayMode
                model: [
                    {
                        text: "Keep Aspect Radio",
                        value: Common.DisplayMode.Aspect
                    },
                    {
                        text: "Scale and Crop",
                        value: Common.DisplayMode.Crop
                    },
                    {
                        text: "Scale to Fill",
                        value: Common.DisplayMode.Scale
                    },
                ]
                textRole: "text"
                onActivated: cfg_DisplayMode = Common.cbCurrentValue(displayMode)
                Component.onCompleted: currentIndex = Common.cbIndexOfValue(displayMode, cfg_DisplayMode)
            }

            CheckBox {
                id: muteAudio
                Layout.columnSpan: 2
                text: "Mute Audio"
            }          

            CheckBox{
                Layout.columnSpan: 2
                visible: Common.checklib_wallpaper(configCol)
                id: useMpv
                text: "Use mpv"
            }
            
            Label{
                id: fpsLabel
                text: "Fps" 
                ToolTip.visible: fpsMouse.containsMouse
                ToolTip.text: qsTr("Control fps on scene wallpaper")
                MouseArea {
                    id: fpsMouse
                    anchors.fill: parent
                    hoverEnabled: true
                }
            }
            RowLayout {
                Label{
                    text: sliderFps.value.toString()
                }

                Slider {
                    id: sliderFps
                    Layout.fillWidth: true
                    from: 5
                    to: 60
                    stepSize: 1.0
                    snapMode: Slider.SnapOnRelease
                }
            }
            Label{
                visible: !cfg_MuteAudio
                id: volumLabel
                text: "Volum"
                MouseArea {
                    id: volumMouse
                    anchors.fill: parent
                    hoverEnabled: true
                }
            }
            RowLayout {
                visible: !cfg_MuteAudio
                Label {
                    text: sliderVol.value.toString()
                }
                Slider {
                    id: sliderVol
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                    stepSize: 5.0
                    snapMode: Slider.SnapOnRelease
                }
            }

        }
        Item {
            Layout.column: 1
            height: 200
            width: height * (16.0/9.0)
        AnimatedImage {
            id: previewAnim
            anchors.fill: parent
            property var nullItem: QtObject {
                property string source: ""
            }
            property var picItem: (picViewLoader.status == Loader.Ready && picViewLoader.item.view.currentItem)
                                ? picViewLoader.item.view.currentItem
                                : nullItem
            source: ""
            cache: false
            asynchronous: true
            onStatusChanged: playing = (status == AnimatedImage.Ready) 
            onPicItemChanged: {
                if(picItem != nullItem) {
                    parent.height = picItem.thumbnail[1].height * 1.5;
                    source = picItem.thumbnail[1].source;
                }
            }
        }
        }
    }

    Row {
        id: infoRow
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 10
        topPadding: 20

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
                    subText: cfg_SteamLibraryPath?cfg_SteamLibraryPath:"Select steam library dir"
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
            width: refreshButton.width * 1.5
            ListModel {
                id: filterModel
                ListElement { text: "scene"; type:"type"; key:"scene"; value:1 }
                ListElement { text: "web"; type:"type"; key:"web"; value:1 }
                ListElement { text: "video"; type:"type"; key:"video"; value:1 }
                function map(func) {
                    let arr = [];
                    for(let i=0;i<this.count;i++) arr.push(func(this.get(i), i));
                    return arr;
                }
            }
            model: filterModel
            displayText: ""
            indicator: PlasmaCore.IconItem {
                x: comboxFilter.leftPadding
                y: comboxFilter.topPadding + (comboxFilter.availableHeight - height) / 2
                source: "view-filter"
                PlasmaCore.ToolTipArea {
                    anchors.fill: parent
                    subText: ""
                }
            }
            delegate: ItemDelegate {
                RowLayout {
                    CheckBox {
                        text: model.text
                        checked: model.value
                        onToggled: {
                            filterModel.get(index).value = Number(this.checked);
                            cfg_FilterStr = comboxFilter.intArrayToStr(comboxFilter.getModelValueArray());
                            folderWorker.filter();
                        }
                    }
                }
            }
            onActivated: {
            }
            function getModelValueArray() {
                return comboxFilter.model.map((e) => e.value);
            }
            function updateModelValue(arr) {
                arr.map((el, index) => comboxFilter.model.get(index).value = el);
            }
            function strToIntArray(str) {
                return [...str].map((e) => e.charCodeAt(0) - '0'.charCodeAt(0));
            }
            function intArrayToStr(arr) {
                return arr.reduce((acc, e) => acc + e.toString(), "");
            }
            Component.onCompleted: {
                this.updateModelValue(this.strToIntArray(cfg_FilterStr));
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
                action: "filter", 
                data: folderWorker.proxyModel,
                model: projectModel,
                filters: comboxFilter.model.map((el) => {
                    return {
                        type: el.type,
                        key: el.key,
                        value: el.value
                    };
                })
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
                    ?"There are no wallpapers in steam library's workshop directory"
                    :"Select your steam library through the folder selecting button above"
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
        title: "Select steam library dir"
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
