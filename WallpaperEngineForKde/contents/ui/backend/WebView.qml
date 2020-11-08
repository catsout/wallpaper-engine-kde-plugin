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
import QtWebEngine 1.10
WebEngineView {
	anchors.fill: background
	audioMuted: background.mute
	url: background.source
	onContextMenuRequested: function(request) {
		request.accepted = true;
    }
    Component.onCompleted: {
        WebEngine.settings.fullscreenSupportEnabled = true
        WebEngine.settings.printElementBackgrounds = false
        WebEngine.settings.playbackRequiresUserGesture = false
		WebEngine.settings.pdfViewerEnabled = false
		WebEngine.settings.showScrollBars = false
    }

}
