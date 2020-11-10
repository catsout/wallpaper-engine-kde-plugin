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
import com.github.catsout.wallpaperEngineKde 1.0

Item{
    id: videoItem
    anchors.fill: parent
    property real volume: 0

    // logfile
    // source
    // mute
    // volume
    // fun:setProperty(name,value)
    MpvObject {
        id: player
        anchors.fill: parent
        source: background.source
        mute: background.mute
        volume: videoItem.volume
    }
    Component.onCompleted:{
        background.nowBackend = "mpv";
    }   


    function play(){
        // stop pause time to avoid quick switch which cause keep pause 
        pauseTimer.stop();
        if(!background.mute)
            volumeTimer.start();
        else videoItem.volume = 100;
        player.play();
    }
    function pause(){
        // need stop volumeTimer to increase volume
        volumeTimer.stop();
        if(!background.mute){
            // set volume and wait before pause, It's to solve the problem that real volume keep high 
            videoItem.volume = 0;
            pauseTimer.start();
        }
        else player.pause();
    }
    Timer{
        id: volumeTimer
        running: false
        repeat: true
        interval: 300
        onTriggered: {
            // increase volume by time
            if(videoItem.volume >= 99)
            {
                videoItem.volume = 100;
                volumeTimer.stop();
            }
            else
                videoItem.volume += 5;
        }
    }
    Timer{
        id: pauseTimer
        running: false
        repeat: false
        interval: 200
        onTriggered: {
            player.pause();
        }
    }
}
