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

import "checker.js" as Checker

ColumnLayout {
    id: root
    Layout.alignment: Qt.AlignCenter
    
    property string cfg_SteamLibraryPath
    property string cfg_WallpaperWorkShopId
    property string cfg_WallpaperFilePath
    property string cfg_WallpaperType
    property string cfg_BackgroundColor: "black"
    property int  cfg_FillMode: 2
    property int  cfg_PauseMode: 0
    property bool cfg_MuteAudio: true
    property bool cfg_UseMpv: false

    RowLayout {
        id: selectRow
        Layout.alignment: Qt.AlignCenter
        Layout.topMargin: 10.0

        Label {
            text: "Pause Mode:"
            Layout.alignment: Qt.AlignLeft 
        }
        
        ComboBox {
            id: pauseMode
            model: [
                {
                    'label': "Maximied Window",
                },
                {
                    'label': "Any Window",
                },
                {
                    'label': "Never Pause",
                }
            ]
            textRole: "label"
            currentIndex: cfg_PauseMode
            onCurrentIndexChanged: cfg_PauseMode = pauseMode.currentIndex
        }
        
    }
    Text {
        Layout.alignment: Qt.AlignCenter
        text: "No qt-labs-folderlistmodel found, please install it in your linux distro."
        color: "red"
        visible: !Checker.checklib_folderlist(checkRow) 
    }
    Text {
        Layout.alignment: Qt.AlignCenter
        text: "Scene wallpaper may crash kde earily, make sure you know how to fix."
        color: "yellow"
        visible: Checker.checklib_wallpaper(checkRow)
    }
    RowLayout {
        id: checkRow
        Layout.alignment: Qt.AlignCenter
        CheckBox {
            id: muteAudio
            text: "Mute Audio"
            checked: cfg_MuteAudio
            onCheckedChanged: {
                    cfg_MuteAudio = muteAudio.checked;
            }
        }          
        RowLayout{
            visible: Checker.checklib_wallpaper(checkRow)

            Label{
                text: "|"
                Layout.alignment: Qt.AlignLeft 
            }

            CheckBox{
                id: useMpv
                text: "Use mpv"
                checked: cfg_UseMpv 
                onCheckedChanged: {
                    cfg_UseMpv = useMpv.checked;
                }
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
        property var files
        property var name_to_index
        property var lock: false
        folder: cfg_SteamLibraryPath + "/steamapps/workshop/content/431960" 
        onStatusChanged: {
            if (wplist.status == FolderListModel.Ready && cfg_SteamLibraryPath !== "")
            {
                wplist.files = {};
                wplist.name_to_index = {};
                wplist.lock = false;
                projectModel.clear();
                for(var i=0;i < wplist.count;i++)
                {
                    var k = i;
                    var v = {
                        "workshopid": wplist.get(i,"fileName"),
                        "path": wplist.get(i,"filePath"),
                        "loaded": false,
                        "enabled": false,
                        "index": i
                    };
                    wplist.files[k] = v;
                }
                Object.entries(wplist.files).forEach(([k,v]) => {
                    readTextFile(v["path"] + "/project.json",
                            function (text) {
                                //console.log("read project:" + v["workshopid"])
                                var json = JSON.parse(text);    
                                v["title"] = json["title"]?json["title"]:"unknown";
                                v["preview"] = json["preview"];
                                v["file"] = json["file"];
                                v["type"] = json["type"]?json["type"]:"unknown";
                                v["loaded"] = true;
                                v["type"] = v["type"].toLowerCase();
                                check();
                            }
                    );
                });
            }
        }
        function check() {
            for(var k in wplist.files)
                if(!wplist.files[k]["loaded"]) return;
            if(wplist.lock) return;
            wplist.lock = true; 
            var currentIndex = 0;
            for(var i=0;wplist.files.hasOwnProperty(i);i++) {
                var v = wplist.files[i];
                projectModel.append(v);
                wplist.name_to_index[v["workshopid"]] = i;
            }
            picView.view.currentIndex = wplist.name_to_index[cfg_WallpaperWorkShopId];
        }
        function readTextFile(fileUrl, callback){
            var xhr = new XMLHttpRequest;
            xhr.open("GET", fileUrl);
            xhr.onreadystatechange = function () {
                if(xhr.readyState === XMLHttpRequest.DONE){
                    var response = xhr.responseText;
                    if(response)
                        callback(response);
                    else
                        callback("{}");
                }
            }
            xhr.send();
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
               picView.view.currentIndex = wplist.name_to_index[workshopid];
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
            var path = wpDialog.fileUrls[0];
            if(path.slice(-1) === '/')
                path = path.slice(0,-1);
            cfg_SteamLibraryPath = path;
        }
    }

    onCfg_WallpaperTypeChanged: {}
}
