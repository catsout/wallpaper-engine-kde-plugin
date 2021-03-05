import QtQuick 2.5
import QtWebEngine 1.10
WebEngineView {
//WebView {
    id: web
    anchors.fill: parent
    audioMuted: background.mute
    url: background.source
    activeFocusOnPress: false
    
    onContextMenuRequested: function(request) {
        request.accepted = true;
    }
    Component.onCompleted: {
        WebEngine.settings.fullscreenSupportEnabled = true
        WebEngine.settings.printElementBackgrounds = false
        WebEngine.settings.playbackRequiresUserGesture = false
        WebEngine.settings.pdfViewerEnabled = false
        WebEngine.settings.showScrollBars = false
        background.nowBackend = "QtWebEngine";
    }
    function play(){
    }
    function pause(){
    }
    function setMouseListener(){
    }
}
