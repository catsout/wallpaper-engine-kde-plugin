/*
 *  Copyright 2020 catsout  <outl941@163.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

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
    
    property string cfg_SteamWorkShopPath
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
                }
            ]
            textRole: "label"
            Component.onCompleted: pauseMode.currentIndex = wallpaper.configuration.PauseMode
            onCurrentIndexChanged: cfg_PauseMode = pauseMode.currentIndex
        }
        
    }
    Text {
        Layout.alignment: Qt.AlignCenter
        text: "No qt-labs-folderlistmodel found, please install it in your linux distro."
        color: "red"
        visible: !Checker.checklib_folderlist(checkRow) 
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
                    subText: cfg_SteamWorkShopPath?cfg_SteamWorkShopPath:"Select steam workshop dir"
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
        folder: cfg_SteamWorkShopPath + "/content/431960" 
        onStatusChanged: {
            if (wplist.status == FolderListModel.Ready && cfg_SteamWorkShopPath !== "")
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
                        "load": false,
                        "index": i
                    };
                    wplist.files[k] = v;
                }
                Object.entries(wplist.files).forEach(([k,v]) => {
                    console.log(v["path"])
                    readTextFile(v["path"] + "/project.json",
                            function (text) {
                                var json = JSON.parse(text);    
                                v["title"] = json["title"];
                                v["preview"] = json["preview"];
                                v["file"] = json["file"];
                                v["type"] = json["type"].toLowerCase();
                                v["load"] = true;
                                check();
                            }
                    );
                });
            }
        }
        function check() {
            for(var k in wplist.files)
                if(!wplist.files[k]["load"]) return;
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
                    callback(response)
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
        title: "Select steam workshop dir"
        selectFolder: true
        selectMultiple : false
        nameFilters: [ "All files (*)" ]
        onAccepted: {
            cfg_SteamWorkShopPath = wpDialog.fileUrls[0]
        }
    }

    onCfg_WallpaperTypeChanged: {}
}
