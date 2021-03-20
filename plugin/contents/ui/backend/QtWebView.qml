import QtQuick 2.5
import QtWebEngine 1.10
import QtWebChannel 1.10
import ".."

Item {
    id: webItem
    anchors.fill: parent
    property bool hasLib: Common.checklib_wallpaper(webItem)
    property int fps: background.fps

    onFpsChanged: {
        if(webobj.loaded) {
            webobj.generalProperties.fps = webItem.fps;
            webobj.sigGeneralProperties(webobj.sigGeneralProperties);
        }
    }

    Image {
        id: pauseImage
        anchors.fill: parent
        visible: true
        enabled: false
    }
    QtObject {
        id: webobj
        WebChannel.id: "wpeQml"
        signal sigGeneralProperties(var properties)
        signal sigUserProperties(var properties)
        signal sigAudio(var audioArray)
        property bool loaded: false
        property var userProperties 
        property var generalProperties
        onLoadedChanged: {
            if(!webobj.generalProperties)
                webobj.generalProperties = {fps: 24};
            webobj.sigGeneralProperties(webobj.generalProperties);
            Common.readTextFile(background.getWorkshopPath() + "/project.json", function(text) { 
                let json = Common.parseJson(text);
                webobj.userProperties = json.general.properties;
                webobj.sigUserProperties(webobj.userProperties);
            });
        }
    }
    WebChannel {
        id: channel
        registeredObjects: [webobj]
    }

    WebEngineView {
    //WebView {
        id: web
        anchors.fill: parent
        enabled: true
        audioMuted: background.mute
        url: background.source
        activeFocusOnPress: false
        webChannel: channel
        userScripts: [
            WebEngineScript {
                injectionPoint: WebEngineScript.DocumentCreation
                worldId: WebEngineScript.MainWorld
                name: "QWebChannel"
                sourceUrl: "qrc:///qtwebchannel/qwebchannel.js"
            },
            WebEngineScript {
                injectionPoint: WebEngineScript.DocumentCreation
                worldId: WebEngineScript.MainWorld
                name: "Audio"
                sourceCode: `
                    window.wallpaperRegisterAudioListener = function(listener) {
                        if(window.wpeQml)
                            window.wpeQml.sigAudio.connect(listener);
                        else
                            window.wallpaperRAed = listener;
                    }
                `
            },
            WebEngineScript {
                worldId: WebEngineScript.MainWorld
                injectionPoint: WebEngineScript.Deferred
                name: "ObjectInjector"
                sourceCode: `
                    new QWebChannel(qt.webChannelTransport, function(channel) {
                        window.wpeQml = channel.objects.wpeQml;
                        let wpeQml = window.wpeQml;
                        let propertyListener = window.wallpaperPropertyListener;
                        if(window.wallpaperRAed)
                            wpeQml.sigAudio.connect(window.wallpaperRAed);
                        if(propertyListener) {
                            if(propertyListener.applyGeneralProperties)
                                wpeQml.sigGeneralProperties.connect(propertyListener.applyGeneralProperties);
                            if(propertyListener.applyUserProperties)
                                wpeQml.sigUserProperties.connect(propertyListener.applyUserProperties);
                        }
                        wpeQml.loaded = true;
                    });`
            }
        ]

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
                pauseTimer.start();
            }
            else {
                web.visible = true;
                web.lifecycleState = WebEngineView.LifecycleState.Active;
                pauseImage.visible = false;
            }
        }

        Component.onCompleted: {
            WebEngine.settings.fullscreenSupportEnabled = true;
            WebEngine.settings.autoLoadIconsForPage = false;
            WebEngine.settings.printElementBackgrounds = false;
            WebEngine.settings.playbackRequiresUserGesture = false;
            WebEngine.settings.pdfViewerEnabled = false;
            WebEngine.settings.showScrollBars = false;

            WebEngine.settings.localContentCanAccessRemoteUrls = true

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
            // only check paused status on timer, not set
            // this is async
            web.grabToImage(function(result) {
                // check for paused again, make sure web is visible
                if(web.paused == false || web.visible == false) return;
                pauseImage.source = result.url;
                pauseImage.visible = true;
                web.visible = false;
                web.lifecycleState = WebEngineView.LifecycleState.Frozen;
            });
        }   
    }
    property var mg
    Component.onCompleted: {
        if(webItem.hasLib) {
            webItem.mg = Qt.createQmlObject(`import QtQuick 2.5;
                    import com.github.catsout.wallpaperEngineKde 1.0
                    MouseGrabber {
                        id: mg
                        anchors.fill: parent
                        target: web.children[0] ? web.children[0] : null
                    }`, webItem);
        }
    }

    function play(){
        web.paused = false;
    }
    function pause(){
        // Set status first
        web.paused = true;
    }
    function setMouseListener(){
        if(web.activeFocusOnPress) {
            web.activeFocusOnPress = false;
            if(webItem.mg)
                webItem.mg.captureMouse = false;
        }
        else {
            web.activeFocusOnPress = true;
            if(webItem.mg)
                webItem.mg.captureMouse = true;
        }
    }
}
