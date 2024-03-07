import QtQuick 2.5
import QtMultimedia
import ".."

Item{
    id: videoItem
    anchors.fill: parent
    property alias source: player.source
    property int displayMode: background.displayMode
    property var volumeFade: Common.createVolumeFade(
        videoItem, 
        Qt.binding(function() { return background.mute ? 0 : background.volume; }),
        (volume) => { audioOut.volume = volume / 100.0; }
    )

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
    }
    AudioOutput {
        id: audioOut
        volume: 0.0
        muted: background.mute
    }
    MediaPlayer {
        id: player
        loops: MediaPlayer.Infinite
        playbackRate: background.speed
        videoOutput: videoView
        audioOutput: audioOut
    }
    Component.onCompleted:{
        background.nowBackend = "QtMultimedia";
        videoItem.displayModeChanged();
    }

    function play(){
        pauseTimer.stop();
        player.play();
        volumeFade.start();
    }
    function pause(){
        volumeFade.stop();
        pauseTimer.start();
    }
    Timer{
        id: pauseTimer
        running: false
        repeat: false
        interval: 300
        onTriggered: {
            player.pause();
        }
    }
    function getMouseTarget() {
    }
}
