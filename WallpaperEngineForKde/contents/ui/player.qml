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

//http://doc.qt.io/qt-5/qml-qtmultimedia-video.html
//https://doc.qt.io/qt-5/qml-qtmultimedia-playlist.html

import QtQuick 2.7
//import QtMultimedia 5.14
import org.kde.plasma.core 2.0 as PlasmaCore

Rectangle {
    id: background
    anchors.fill: parent
    color: wallpaper.configuration.BackgroundColor
	property string source: wallpaper.configuration.WallpaperFilePath
	property string type: wallpaper.configuration.WallpaperType
	property bool mute: wallpaper.configuration.MuteAudio
	Timer {
             id: playtimer
             running: true
             repeat: false
             interval: 4000
             onTriggered: {
				 if( background.type == "video") backend.play()
             }
    }

//	MediaPlayer {}
//	VideoOutput {}

//	Component {
//		id: mpv
//		MpvPlayer {}
//	}
	property var backend
	property var backComponent
	function createBackend(){
		// choose backend
		switch (background.type) {
			case 'video':
				backComponent = Qt.createComponent("backend/multimedia.qml");
				//backComponent = Qt.createComponent("backend/mpv.qml");
				break;
			case 'web':
				backComponent = Qt.createComponent("backend/WebView.qml")
				break;
			default:
				return;
		}

		if (backComponent.status == Component.Error)
			console.error("Error loading component:",backComponent.errorString())
		if (backComponent.status == Component.Ready || backComponent.status == Component.Error)
			loadBackend()
		else
			backComponent.statusChanged.connect(loadBackend)
	}
	function loadBackend(){
		var paraWebView = {
			"url": background.source
		}
		var paraVideo = {
			"video_source": background.source
		}
		// choose para
		var para = null
		switch (background.type) {
			case 'video':
				para = paraVideo
				break;
			case 'web':
				para = paraWebView
				break;
		}
		backend = backComponent.createObject(background, para)
		
		//if ( background.type == "video" ) backend.initFinished.connect(mpvFinish)
		function mpvFinish(){
            backend.pause()
		}
	}
	
	function typeCallback() {
		var old = backend
		backComponent = null
		createBackend()
		connectOk()
		old.destroy()
	}

	function sourceCallback() {
        if ( background.type == "video" ) backend.video_source = background.source
        else if ( background.type == "web" ) backend.url = background.source
	}
    
	Component.onCompleted: {
		createBackend()
		background.sourceChanged.connect(sourceCallback)
		background.typeChanged.connect(typeCallback)
		connectOk()
	}

	property bool ok: windowModel.playVideoWallpaper
	function connectOk(){
		if (background.type == "video")
			background.okChanged.connect(autoPause)
		else background.okChanged.disconnect(autoPause)

	}
	function autoPause() {background.ok?backend.play():backend.pause()}
    WindowModel {
        id: windowModel
    }
}
/*
    MediaPlayer {
        id: playlistplayer
        autoPlay: true
		loops: MediaPlayer.Infinite
        muted: wallpaper.configuration.MuteAudio
        //bufferProgress: 0.7
        source: wallpaper.configuration.WallpaperFilePath 
    }

    VideoOutput {
        id: videoView
        //visible: false
        fillMode: wallpaper.configuration.FillMode
        anchors.fill: parent
        source: playlistplayer
        flushMode: VideoOutput.LastFrame 
    }
*/
