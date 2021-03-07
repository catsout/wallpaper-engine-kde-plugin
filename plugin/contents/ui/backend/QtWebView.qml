import QtQuick 2.5
import QtWebEngine 1.10
Item {
    id: webItem
    anchors.fill: parent
    Image {
        id: pauseImage
        property bool paused: false
        anchors.fill: parent
        visible: true
        enabled: false
    }
    WebEngineView {
    //WebView {
        id: web
        anchors.fill: parent
        enabled: true
        audioMuted: background.mute
        url: background.source
        activeFocusOnPress: false

        property bool paused: false
        
        //onContextMenuRequested: function(request) {
        //    request.accepted = true;
        //}
        onLoadingChanged: {
            if(loadRequest.status == WebEngineView.LoadSucceededStatus) {
                // check pause after load
                if(paused) {
                    webItem.play();
                    webItem.pause();
                }
            }
        }

        onPausedChanged: {
            if(paused) {
                web.grabToImage(function(result) {
                    // check for paused changed again
                    if(web.visible == false || web.visible == false) return;
                    pauseImage.source = result.url;
                    pauseImage.visible = true;
                    web.visible = false;
                    web.lifecycleState = WebEngineView.LifecycleState.Frozen;
                });
            }
            else {
                web.visible = true;
                web.lifecycleState = WebEngineView.LifecycleState.Active;
                pauseImage.visible = false;
            }
        }

        Component.onCompleted: {
            WebEngine.settings.fullscreenSupportEnabled = true
            WebEngine.settings.printElementBackgrounds = false
            WebEngine.settings.playbackRequiresUserGesture = false
            WebEngine.settings.pdfViewerEnabled = false
            WebEngine.settings.showScrollBars = false
            background.nowBackend = "QtWebEngine";
        }

    }
    // There is no signal for frame complete, so use timer to make sure not black result
    Timer{
        id: pauseTimer
        running: false
        repeat: false
        interval: 300 
        onTriggered: {
            web.paused = true;
        }   
    }   
    function play(){
        web.paused = false;
    }
    function pause(){
        pauseTimer.start();
    }
    function setMouseListener(){
    }
}
