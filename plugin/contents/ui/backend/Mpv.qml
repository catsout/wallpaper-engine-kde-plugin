import QtQuick 2.5
import com.github.catsout.wallpaperEngineKde 1.0
import ".."

Item{
    id: videoItem
    anchors.fill: parent
    property real volume: 0
    property int displayMode: background.displayMode
    
    onDisplayModeChanged: {
        var value = -1;
        if(displayMode == Common.DisplayMode.Scale)
            value = "no";
        else if(displayMode == Common.DisplayMode.Aspect)
            value = -1;
        player.setProperty("video-aspect-override", value);
    }

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
    function setMouseListener(){
    }  
}
