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
    property int  cfg_SortMode
    property alias cfg_Volume: sliderVol.value
    property alias cfg_FilterMode: comboxFilter.currentIndex

    property alias cfg_MuteAudio: muteAudio.checked
    property alias cfg_UseMpv: useMpv.checked
    property alias cfg_RandomizeWallpaper: randomizeWallpaper.checked
    property alias cfg_MouseInput: mouseInput.checked

    property alias cfg_Fps: sliderFps.value
    property alias cfg_SwitchTimer: randomSpin.value

    property string cfg_CustomConf
    property var customConf: {
        customConf = Common.loadCustomConf(cfg_CustomConf);
    }

    property var iconSizes: {
        if(PlasmaCore.Units) {
            iconSizes = PlasmaCore.Units.iconSizes;
        } else {
            iconSizes = {
                large: 48
            }
        }
    }
    property var themeWidth: {
        if(PlasmaCore.Theme) {
            themeWidth = PlasmaCore.Theme.mSize(theme.defaultFont).width;
        } else if(theme) {
            themeWidth = theme.mSize(theme.defaultFont).width;
        } else {
            themeWidth = font.pixelSize;
        }
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
                    sortMode: cfg_SortMode
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
        implicitWidth: height * 2.125
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
            RowLayout {
                id: infoRow
                Layout.alignment: Qt.AlignHCenter
                spacing: 10
                Label {
                    id: workshopidLabel
                    text: "Shopid: " + cfg_WallpaperWorkShopId
                }
                Label {
                    text: "Type: " + cfg_WallpaperType
                }
                Button {
                    id: wpFolderButton
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
                    implicitWidth: height * 1.5
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
                ComboBox {
                    id: comboxSort
                    Layout.preferredWidth: contentItem.implicitWidth + height
                    popup.width: font.pixelSize * 18

                    model: [
                        {
                            text: "Sort By Workshop Id",
                            short: "Id",
                            value: Common.SortMode.Id
                        },
                        {
                            text: "Sort Alphabetically By Name",
                            short: "Alphabetical",
                            value: Common.SortMode.Name
                        },
                        {
                            text: "Show Newest Modified First",
                            short: "Modified",
                            value: Common.SortMode.Modified
                        }
                    ]
                    contentItem: RowLayout { 
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        spacing: 0
                        Item {
                            Layout.fillHeight: true
                            Layout.preferredWidth: height
                            PlasmaCore.IconItem {
                                anchors.fill: parent
                                source: "view-sort-descending-symbolic"
                            }
                        }
                        Label {
                            Layout.fillHeight: true
                            text: comboxSort.model[comboxSort.currentIndex].short
                        }
                        Item { Layout.fillWidth: true ; height: 1 }
                    }
                    ButtonGroup { id: sortGroup }
                    delegate: ItemDelegate {
                        width: comboxSort.popup.width
                        contentItem: RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 5
                            PlasmaComponents3.RadioButton {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                text: modelData.text
                                autoExclusive: true
                                ButtonGroup.group: sortGroup
                                onClicked:   {
                                    comboxSort.currentIndex = index;
                                    comboxSort.popup.close();
                                    cfg_SortMode = Common.cbCurrentValue(comboxSort)
                                }
                                checked: {
                                    checked = index == cfg_SortMode;
                                }
                            }
                        }
                    }
                    Component.onCompleted: currentIndex = Common.cbIndexOfValue(this, cfg_SortMode)
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
                                width: root.iconSizes.large
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
                                fillMode: Image.PreserveAspectCrop//Image.Stretch
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
                implicitWidth: themeWidth * 24
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
                implicitWidth: themeWidth * 24
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
                    PlasmaCore.ToolTipArea {
                        anchors.fill: parent
                        subText: "randomize wallpapers showing in the Wallpaper page"
                    }
                }
                RowLayout {
                    enabled: cfg_RandomizeWallpaper
                    Label { 
                        id:heightpicker
                        text: " every " 
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
            CheckBox {
                id: mouseInput
                visible: libcheck.wallpaper
                Kirigami.FormData.label: "Mouse input:"
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
                }
                Slider {
                    id: sliderFps
                    Layout.fillWidth: true
                    from: 5
                    to: 60
                    stepSize: 1.0
                    snapMode: Slider.SnapOnRelease
                    PlasmaCore.ToolTipArea {
                        anchors.fill: parent
                        subText: "Control fps on scene wallpaper"
                    }
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
            twinFormLayouts: parentLayout
            PlasmaComponents3.TextArea {
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
                background: Item {}
            }
            PlasmaComponents3.TextArea {
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
                background: Item {}
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
