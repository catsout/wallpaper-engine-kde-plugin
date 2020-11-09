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
import org.kde.plasma.core 2.0 as PlasmaCore

Rectangle {
    id: background
    anchors.fill: parent
    color: wallpaper.configuration.BackgroundColor
	property string source: wallpaper.configuration.WallpaperFilePath
	property string type: wallpaper.configuration.WallpaperType
	property bool mute: wallpaper.configuration.MuteAudio
	
	// lauch pause time to avoid freezing
	Timer {
		id: lauchPauseTimer
		running: false
        repeat: false
        interval: 300
        onTriggered: {
				backend.pause();
				playTimer.start();
		}
    }
	Timer{
		id: playTimer
		running: false
		repeat: false
		interval: 5000
		onTriggered: { background.autoPause(); }
	}
	
	property var backend
	property var backComponent
	function createBackend(){
		// choose backend
		switch (background.type) {
			case 'video':
				backComponent = Qt.createComponent("backend/QtMultimedia.qml");
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
		// set para to {}, as it can't keep connect. use id in component direct.
		backend = backComponent.createObject(background, {})
		
		// new backend need autopause
		sourceCallback()
	}
	// signal changed
	function typeCallback() {
		var old = backend
		backComponent = null
		createBackend()
		old.destroy()
	}
	  // as always autoplay for refresh lastframe, sourceChange need autoPause
	function sourceCallback() {
		sourcePauseTimer.start();	
	}

	Timer {
		id: sourcePauseTimer
		running: false
        repeat: false
        interval: 200
        onTriggered: background.autoPause();
    }

	Component.onCompleted: {
		// load first backend
		createBackend(); // background signal connect
		background.typeChanged.connect(typeCallback);
		background.sourceChanged.connect(sourceCallback);
		background.okChanged.connect(autoPause);
		lauchPauseTimer.start();
	}
	
	// auto pause
	property bool ok: windowModel.playVideoWallpaper
	function autoPause() {background.ok?backend.play():backend.pause()}
    WindowModel {
        id: windowModel
    }
}
