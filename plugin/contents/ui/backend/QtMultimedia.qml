import QtQuick 2.5
import QtMultimedia 5.13
import ".."

Item{
    id: videoItem
    anchors.fill: parent
    property real volume: 0.0
    property int displayMode: background.displayMode

    onDisplayModeChanged: {
        if(displayMode == Common.DisplayMode.Scale)
            videoView.fillMode = VideoOutput.Stretch;
        else if(displayMode == Common.DisplayMode.Aspect)
            videoView.fillMode = VideoOutput.PreserveAspectFit;
        else if(displayMode == Common.DisplayMode.Crop)
            videoView.fillMode = VideoOutput.PreserveAspectCrop;
    }

    VideoOutput {
        id: videoView
        //fillMode: wallpaper.configuration.FillMode
        anchors.fill: parent
        source: player
        // keep lastframe for loop 
        flushMode: VideoOutput.LastFrame 
    }
    MediaPlayer {
        id: player
        autoPlay: true
        loops: MediaPlayer.Infinite
        muted: background.mute
        source: background.source
        // can't set player.volume direct, use a var
        volume: videoItem.volume
    }
    Component.onCompleted:{
        background.nowBackend = "QtMultimedia";
    }

    function play(){
        // stop pause time to avoid quick switch which cause keep pause 
        pauseTimer.stop();
        if(!background.mute)
            volumeTimer.start();
        else videoItem.volume = 1;
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
            if(videoItem.volume >= 0.8)
            {
                videoItem.volume = 1.0;
                volumeTimer.stop();
            }
            else
                videoItem.volume += 0.05;
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
    function getMouseTarget() {
    }
}
