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
				backendLoder.item.pause();
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
	Loader { 
		id: backendLoder
		anchors.fill: parent
		//sourceComponent: rect
	}
	
	function loadBackend(){
		var qmlsource = "";

		// check source
		if(!background.source || background.source == "") return;
		// choose backend
		switch (background.type) {
			case 'video':
				qmlsource = "backend/QtMultimedia.qml";
				//backComponent = Qt.createComponent("backend/mpv.qml");
				break;
			case 'web':
				qmlsource = "backend/WebView.qml";
				break;
			default:
				return;
		}
		backendLoder.source = qmlsource;
		sourceCallback();
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
		loadBackend(); // background signal connect
		background.typeChanged.connect(loadBackend);
		background.sourceChanged.connect(sourceCallback);
		background.okChanged.connect(autoPause);
		lauchPauseTimer.start();
	}
	
	// auto pause
	property bool ok: windowModel.playVideoWallpaper
	function autoPause() {background.ok
					? backendLoder.item.play()
					: backendLoder.item.pause()
	}
	
    WindowModel {
        id: windowModel
    }

}
