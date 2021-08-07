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
    property alias cfg_RandomizeWallpaper: randomizeWallpaper.checked

    property alias cfg_Fps: sliderFps.value
    property alias cfg_SwitchTimer: randomSpin.value

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
            RowLayout {
                Layout.columnSpan: 2
                spacing: 0
                CheckBox{
                    id: randomizeWallpaper
                    text: "Randomize"
                }
                RowLayout {
                    visible: cfg_RandomizeWallpaper
                    Text { id:heightpicker; text: " every " }
                    SpinBox {
                        id: randomSpin
                        from: 1
                        to: 120
                        stepSize: 1
                    }
                    Text { text: " min" }
                }
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
                Text {
                    Layout.preferredWidth: font.pixelSize * 2
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
                text: "Volume"
            }
            RowLayout {
                visible: !cfg_MuteAudio
                Text {
                    Layout.preferredWidth: font.pixelSize * 2
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
            height: 180
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
                        try {
                            source = picItem.thumbnail[1].source;
                        } catch(e) {}
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
        ComboBox {
            id: comboxFilter
            anchors.verticalCenter: parent.verticalCenter
            width: wpFolderButton.width * 1.5

            property var modelValues: Common.filterModel.getValueArray(cfg_FilterStr)
            model: Common.filterModel

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
                        checked: comboxFilter.modelValues[index]
                        onToggled: {
                            const modelValues = comboxFilter.modelValues;
                            modelValues[index] = Number(this.checked);
                            cfg_FilterStr = Common.intArrayToStr(modelValues);
                        }
                    }
                }
            }
            onActivated: {
            }
        }
    }

    WallpaperListModel {
        id: wpListModel
        workshopDir: cfg_SteamLibraryPath + Common.wpenginePath
        filterStr: cfg_FilterStr
        enabled: Boolean(cfg_SteamLibraryPath)
    }

    Loader {
        id: picViewLoader
        asynchronous: true
        sourceComponent: picViewCom
        visible: status == Loader.Ready

        height: root.height - configRow.height - infoRow.height - warnRow.height - root.spacing*3
        anchors.left: parent.left
        anchors.right: parent.right

        Component.onCompleted: {
            const refreshIndex = () => {
                if(picViewLoader.status == Loader.Ready)
                    picViewLoader.item.setCurIndex(wpListModel.model);
            }
            wpListModel.modelRefreshed.connect(refreshIndex);
            picViewLoader.statusChanged.connect(refreshIndex);
        }
    }
    Component { 
        id: picViewCom
        KCM.GridView {
            id: picViewGrid
            anchors.fill: parent

            view.model: wpListModel.model
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

            function setCurIndex(model) {
                // model, ListModel
                new Promise((reoslve, reject) => {
                    for(let i=0;i < model.count;i++) {
                        if(model.get(i).workshopid === cfg_WallpaperWorkShopId) {
                            view.currentIndex = i;
                            break;
                        }
                    }
                    if(view.currentIndex == -1 && model.count != 0)
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
