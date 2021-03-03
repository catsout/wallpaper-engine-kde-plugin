import QtQuick 2.5
import org.kde.plasma.core 2.0 as PlasmaCore
import "checker.js" as Checker

Rectangle {
    id: background
    anchors.fill: parent
    color: wallpaper.configuration.BackgroundColor
    property string steamlibrary: wallpaper.configuration.SteamLibraryPath
    property string type: wallpaper.configuration.WallpaperType
    property string source: wallpaper.configuration.WallpaperFilePath
    property bool mute: wallpaper.configuration.MuteAudio
    property bool useMpv: wallpaper.configuration.UseMpv
    
    property string nowBackend: ""

    onSourceChanged: {
        if(background.nowBackend === "InfoShow")
            loadBackend();
    }

    // lauch pause time to avoid freezing
    Timer {
        id: lauchPauseTimer
        running: false
        repeat: false
        interval: 300
        onTriggered: {
                backendLoder.item.pause();
                playTimer.start();
        }
    }
    Timer{
        id: playTimer
        running: false
        repeat: false
        interval: 5000
        onTriggered: { background.autoPause(); }
    }
    // lauch pause end
    
    // main  
    Loader { 
        id: backendLoder
        anchors.fill: parent
    }
    Component.onCompleted: {
        // load first backend
        loadBackend(); // background signal connect
        background.typeChanged.connect(loadBackend);
        background.useMpvChanged.connect(loadBackend);
        background.sourceChanged.connect(sourceCallback);
        background.okChanged.connect(autoPause);
        lauchPauseTimer.start();
    }
    
    
    function loadBackend(){
        var qmlsource = "";
        var properties = {};

        // check source
        if(!background.source || background.source == "") {
            qmlsource = "backend/InfoShow.qml";
            properties = {"info":"Error: source is empty.\n The config may be broken."};
            backendLoder.setSource(qmlsource, properties);
            return;
        }
        // choose backend
        switch (background.type) {
            case 'video':
                if(background.useMpv && Checker.checklib_wallpaper(background))
                    qmlsource = "backend/Mpv.qml";
                else qmlsource = "backend/QtMultimedia.qml";
                break;
            case 'web':
                qmlsource = "backend/QtWebView.qml";
                break;
            case 'scene':
                if(Checker.checklib_wallpaper(background)) {
                    qmlsource = "backend/Scene.qml";
                    properties = {"assets": background.steamlibrary + "/steamapps/common/wallpaper_engine/assets"};
                } else {
                    qmlsource = "backend/InfoShow.qml";
                    properties = {"info": "Error: plugin lib not found,\nscene support require to compile and install it."};
                }
                break;
            default:
                qmlsource = "backend/InfoShow.qml";
                properties = {"info":"Not supported wallpaper type: "+background.type};
                break
        }
        console.log("load backend: "+qmlsource);
        backendLoder.setSource(qmlsource, properties);
        sourceCallback();
    }
    
    // As always autoplay for refresh lastframe, sourceChange need autoPause
    // need a time for delay, which is needed for refresh
    function sourceCallback() {
        sourcePauseTimer.start();   
    }
    Timer {
        id: sourcePauseTimer
        running: false
        repeat: false
        interval: 200
        onTriggered: background.autoPause();
    }

   
    // auto pause
    property bool ok: windowModel.playVideoWallpaper
    function autoPause() {background.ok
                    ? backendLoder.item.play()
                    : backendLoder.item.pause()
    }
    WindowModel {
        id: windowModel
    }

}
