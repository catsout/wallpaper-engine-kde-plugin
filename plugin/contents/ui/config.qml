import QtQuick 2.6
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.5
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
// for kcm gridview
import org.kde.kcm 1.1 as KCM
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kquickcontrolsaddons 2.0

import Qt.labs.folderlistmodel 2.11
import "utils.mjs" as Utils

ColumnLayout {
    id: root
    anchors.fill: parent
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

    property string cfg_CustomConf
    property var customConf: {
        customConf = Common.loadCustomConf(cfg_CustomConf);
    }
    property var libcheck: ({
        wallpaper: Common.checklib_wallpaper(root),
        folderlist: Common.checklib_folderlist(root),
        qtwebsockets: Common.checklib_websockets(root),
        qtwebchannel: Common.checklib_webchannel(root)
    })
    property var pyext: {
        if(!libcheck.qtwebsockets) {
            pyext = null
        } else {
            pyext = Qt.createQmlObject(`
                import QtQuick 2.0;
                Pyext {}
            `, this);
        }
    }
    property var wpListModel: {
        if(!libcheck.folderlist) { 
            wpListModel = null;
        } else {
            wpListModel = Qt.createQmlObject(`
                import QtQuick 2.0;
                WallpaperListModel {
                    workshopDirs: Common.getProjectDirs(cfg_SteamLibraryPath)
                    filterStr: cfg_FilterStr
                    initItemOp: (item) => {
                        if(!root.customConf) return;
                        item.favor = root.customConf.favor.has(item.workshopid);
                    }
                    enabled: Boolean(cfg_SteamLibraryPath)
                    readfile: pyext.readfile
                }
            `, this);
        }
    }

    Component.onDestruction: {
        if(this.pyext) this.pyext.destroy();
        if(this.wpListModel) this.wpListModel.destroy();
    }

    function saveCustomConf() {
        cfg_CustomConf = Common.prepareCustomConf(this.customConf);
    }

    PlasmaComponents3.TabBar {
        id: bar
        //currentTab
        PlasmaComponents3.TabButton {
            text: qsTr("Wallpapers")
        }
        PlasmaComponents3.TabButton {
            text: qsTr("Settings")
        }
        PlasmaComponents3.TabButton {
            text: qsTr("About")
        }
    }


    StackLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: bar.currentIndex
        ColumnLayout {
            id: wpselsect
            Layout.fillWidth: true
            Row {
                id: infoRow
                Layout.alignment: Qt.AlignHCenter
                spacing: 10
                Label {
                    id: workshopidLabel
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Shopid: " + cfg_WallpaperWorkShopId
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
                    width: height * 1.5
                    popup.width: font.pixelSize * 11
                    popup.height: font.pixelSize * 25

                    property var modelValues: Common.filterModel.getValueArray(cfg_FilterStr)
                    model: Common.filterModel

                    indicator: PlasmaCore.IconItem {
                        height: parent.height
                        width: height
                        source: "filter-symbolic"
                        PlasmaCore.ToolTipArea {
                            anchors.fill: parent
                            subText: ""
                        }
                    }
                    delegate: ItemDelegate {
                        id: combox_dg
                        width: comboxFilter.popup.width
                        contentItem: RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 5
                            CheckBox {
                                visible: model.type !== "_nocheck"
                                checked: comboxFilter.modelValues[index]
                                onToggled: combox_dg.toggle()
                            }
                            Label {
                                Layout.fillWidth: true
                                text: model.text
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: parent.toggle()
                        }
                        function toggle() {
                            if(model.type === "_nocheck") return;
                            const modelValues = comboxFilter.modelValues;
                            modelValues[index] = Number(!modelValues[index]);
                            cfg_FilterStr = Utils.intArrayToStr(modelValues);
                        }
                    }
                    
                }
            }

            Loader {
                id: picViewLoader
                Layout.fillWidth: true
                Layout.fillHeight: true

                asynchronous: false
                sourceComponent: picViewCom
                visible: status == Loader.Ready

                Component.onCompleted: {
                    const refreshIndex = () => {
                        if(this.status == Loader.Ready) {
                            this.item.setCurIndex(wpListModel.model);
                        }
                    }
                    wpListModel.modelStartSync.connect(this.item.backtoBegin);
                    wpListModel.modelRefreshed.connect(refreshIndex.bind(this));
                }
            }
            Component { 
                id: picViewCom
                KCM.GridView {
                    id: picViewGrid
                    anchors.fill: parent

                    view.model: wpListModel.model
                    view.delegate: KCM.GridDelegate {
                        // path is file://, safe to concat with '/'
                        text: title
                        actions: [
                            Kirigami.Action {
                                icon.name: favor?"user-bookmarks-symbolic":"bookmark-add-symbolic"
                                tooltip: favor?"Remove from favorites":"Add to favorites"
                                onTriggered: {
                                    if(favor) {
                                        root.customConf.favor.delete(workshopid);
                                    } else {
                                        root.customConf.favor.add(workshopid);
                                    }
                                    view.model.assignModel(index, { favor: !favor });
                                    root.saveCustomConf();
                                }
                            },
                            Kirigami.Action {
                                icon.name: "folder-remote-symbolic"
                                tooltip: "Open Workshop Link"
                                visible: Boolean(workshopid)
                                onTriggered: Qt.openUrlExternally("https://steamcommunity.com/sharedfiles/filedetails/?id=" + workshopid)
                            },
                            Kirigami.Action {
                                icon.name: "document-open-folder"
                                tooltip: "Open Containing Folder"
                                onTriggered: Qt.openUrlExternally(path) 
                            }
                        ]
                        thumbnail: Rectangle {
                            anchors.fill: parent
                            QIconItem {
                                anchors.centerIn: parent
                                width: PlasmaCore.Units.iconSizes.large
                                height: width
                                icon: "view-preview"
                                visible: !imgPre.visible
                            }
                            Image {
                                id: imgPre
                                anchors.fill: parent
                                source: preview?path + "/" + preview:""
                                sourceSize.width: parent.width
                                sourceSize.height: parent.height
                                fillMode: Image.Stretch
                                cache: false
                                asynchronous: true
                                visible: Boolean(preview)
                            }
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
                        text: { 
                            if(!libcheck.folderlist)
                                return `Please make sure qt-labs-folderlistmodel installed, and open this again`;
                            if(!(libcheck.qtwebsockets && pyext))
                                return `Please make sure qtwebsockets(qml module) installed, and open this again`
                            if(!pyext.ok) {
                                return `Python helper run failed: ${pyext.log}`;
                            }
                            if(!cfg_SteamLibraryPath)
                                return "Select your steam library through the folder selecting button above";
                            if(wpListModel.countNoFilter > 0)
                                return `Found ${wpListModel.countNoFilter} wallpapers, but none of them matched filters`;
                            return `There are no wallpapers in steam library's workshop directory`;
                        }
                        opacity: 0.5
                    }
                    function backtoBegin() {
                        view.positionViewAtBeginning();
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
                title: "Select steamlibrary folder"
                selectFolder: true
                selectMultiple : false
                nameFilters: [ "All files (*)" ]
                onAccepted: {
                    const path = Utils.trimCharR(wpDialog.fileUrls[0], '/');
                    cfg_SteamLibraryPath = path;
                    wpListModel.refresh();
                }
            }


        }
        Kirigami.FormLayout {
            id: settingTab
            Layout.fillWidth: true
            twinFormLayouts: parentLayout
            ComboBox {
                Kirigami.FormData.label: "Pause:"
                implicitWidth: PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 24
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
            ComboBox {
                id: displayMode
                Kirigami.FormData.label: "Display:"
                implicitWidth: PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 24
                model: [
                    {
                        text: "Keep Aspect Ratio",
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
                Kirigami.FormData.label: "Mute Audio:"
            }
            RowLayout {
                spacing: 0
                Kirigami.FormData.label:  "Randomize:"
                CheckBox{
                    id: randomizeWallpaper
                }
                RowLayout {
                    enabled: cfg_RandomizeWallpaper
                    ToolTip.visible: randomMouse.containsMouse
                    ToolTip.text: qsTr("randomize wallpapers showing in the Wallpaper page")
                    Label { 
                        id:heightpicker
                        text: " every " 
                        MouseArea {
                            id: randomMouse
                            anchors.fill: parent
                            hoverEnabled: true
                        }
                    }
                    SpinBox {
                        id: randomSpin
                        width: font.pixelSize * 4
                        height: heightpicker.height
                        from: 1
                        to: 120
                        stepSize: 1
                    }
                    Label { text: " min" }
                }
            }
            CheckBox{
                visible: libcheck.wallpaper
                Kirigami.FormData.label:  "Use mpv:"
                id: useMpv
            }
            RowLayout {
                Kirigami.FormData.label:  "Fps:"
                Layout.preferredWidth: displayMode.width
                Label {
                    Layout.preferredWidth: font.pixelSize * 2
                    text: sliderFps.value.toString()
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
                    Layout.fillWidth: true
                    from: 5
                    to: 60
                    stepSize: 1.0
                    snapMode: Slider.SnapOnRelease
                }
            }
            RowLayout {
                Kirigami.FormData.label:  "Volume:"
                visible: !cfg_MuteAudio
                Layout.preferredWidth: displayMode.width
                Label {
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
            /*
            Item {
                Layout.column: 1
                height: 180
                width: height * (16.0/9.0)
                AnimatedImage {
                    id: previewAnim
                    anchors.fill: parent
                    property var picIndex: (picViewLoader.status == Loader.Ready && picViewLoader.item.view.currentIndex >= 0)
                                        ? picViewLoader.item.view.currentIndex
                                        : 0
                    source: ""
                    cache: false
                    asynchronous: true
                    onStatusChanged: playing = (status == AnimatedImage.Ready) 
                    onPicIndexChanged: {
                        const m = wpListModel.model.get(picIndex);
                        if(m) source = m.path + "/" + m.preview;
                    }
                }
            }
            */
        }
        Kirigami.FormLayout {
            Layout.fillWidth: true
            twinFormLayouts: settingTab

            Label {
                Layout.fillWidth: true
                Kirigami.FormData.labelAlignment: Qt.AlignTop
                Kirigami.FormData.label: "Requirements:"
                text: `
                    <ol>
                    <li><i>Wallpaper Engine</i> installed on Steam</li>
                    <li>Subscribe to some wallpapers on the Workshop</li>
                    <li>Select the <i>steamlibrary</i> folder on the Wallpapers tab
                        <ul>
                            <li>The <i>steamlibrary</i> which contains the <i>steamapps<i/> folder</li>
                            <li><i>Wallpaper Engine</i> needs to be installed in this folder</li>
                        </ul>
                    </li>
                    </ol>
                `
                wrapMode: Text.Wrap
                textFormat: Text.RichText
            }
            PlasmaComponents3.TextArea {
                Layout.fillWidth: true
                Kirigami.FormData.label: "Fix crashes:"
                Kirigami.FormData.labelAlignment: Qt.AlignTop
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
                background: Item {}
            }
            ListView {
                Layout.fillWidth: true
                Kirigami.FormData.label: "Lib check:"
                Kirigami.FormData.labelAlignment: Qt.AlignTop
                Layout.preferredHeight: 0
                implicitHeight: (font.pixelSize * 2) * modelraw.length
                model: ListModel {}
                clip: false
                property var modelraw: {
                    const _model = [
                        {
                            ok: libcheck.wallpaper,
                            name: "plugin lib"
                        },
                        {
                            ok: libcheck.folderlist,
                            name: "qt-lab-folderlist"
                        },
                        {
                            ok: libcheck.qtwebchannel,
                            name: "qtwebchannel (qml)"
                        },
                        {
                            ok: libcheck.qtwebsockets,
                            name: "qtwebsockets (qml)"
                        },
                        {
                            ok: pyext && pyext.ok,
                            name: "python3 (python3-websockets)"
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
    }
    onCfg_WallpaperTypeChanged: {}
}
