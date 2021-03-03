import QtQuick 2.5
import com.github.catsout.wallpaperEngineKde 1.0

Item{
    id: sceneItem
    anchors.fill: parent
    property string source: background.source
    property string assets: "assets"

    SceneViewer {
        id: player
        anchors.fill: parent
        source: ""
        assets: sceneItem.assets
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
}
