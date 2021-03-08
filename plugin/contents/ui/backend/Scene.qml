import QtQuick 2.5
import com.github.catsout.wallpaperEngineKde 1.0
import ".."

Item{
    id: sceneItem
    anchors.fill: parent
    property string source: background.source
    property int displayMode: background.displayMode
    property string assets: "assets"

    onDisplayModeChanged: {
        if(displayMode == Common.DisplayMode.Aspect)
            player.keepAspect = true;
        else
            player.keepAspect = false;
    }

    SceneViewer {
        id: player
        anchors.fill: parent
        source: ""
        assets: sceneItem.assets
    }

    MouseGrabber {
        id: mg
        anchors.fill: parent
        target: player
    }

    Component.onCompleted: {
        background.nowBackend = "scene";
    }
    onSourceChanged: {
        var source_ = sceneItem.source;
        player.source = source_.substr(0, source_.length-5) + ".pkg";
    }
    function play() {
        player.play();
    }
    function pause() {
        player.pause();
    }
    
    function setMouseListener(){
        if(mg.captureMouse) {
            player.setAcceptMouse(false);
            mg.captureMouse = false;
        }
        else {
            player.setAcceptMouse(true);
            mg.captureMouse = true;
        }
    }
}
