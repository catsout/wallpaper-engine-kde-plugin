import QtQuick 2.12
import QtQuick.Window 2.2
import org.kde.plasma.core 2.0 as PlasmaCore

Rectangle {
    id: background
    anchors.fill: parent
    color: wallpaper.configuration.BackgroundColor
    property string steamlibrary: Qt.resolvedUrl(wallpaper.configuration.SteamLibraryPath).toString()
    property string source: Qt.resolvedUrl(wallpaper.configuration.WallpaperFilePath).toString()

    property string workshopid: wallpaper.configuration.WallpaperWorkShopId
    property string type: wallpaper.configuration.WallpaperType

    property int displayMode: wallpaper.configuration.DisplayMode
    property bool mute: wallpaper.configuration.MuteAudio
    property bool useMpv: wallpaper.configuration.UseMpv
    property bool randomizeWallpaper: wallpaper.configuration.RandomizeWallpaper

    property int fps: wallpaper.configuration.Fps
    property int volume: wallpaper.configuration.Volume
    property int switchTimer: wallpaper.configuration.SwitchTimer

    property string filterStr: wallpaper.configuration.FilterStr
    // auto pause
    property bool ok: windowModel.playVideoWallpaper

    property string nowBackend: ""

    property var mouseHooker
    property bool hasLib: Common.checklib_wallpaper(background)


    Component.onDestruction: {
        if(mouseHooker) {
            mouseHooker.destroy();
        }
    }
    
    function getWorkshopPath() {
        return steamlibrary + Common.wpenginePath + '/' + workshopid;
    }
    function getAssetsPath() {
        return steamlibrary + "/steamapps/common/wallpaper_engine/assets"
    }

    onSourceChanged: {
        if(background.nowBackend === "InfoShow")
            loadBackend();
    }

    Timer {
        id: hookTimer
        running: true
        repeat: false
        interval: 2000
        property int tryTimes: 0
        onTriggered: {
            tryTimes++; 
            if(tryTimes >= 10 || !background.hasLib)
                return;
            background.hookMouse();
        }
        Component.onCompleted: {
            background.hookMouse.connect(background.hookMouseSlot);
        }
    }
    signal hookMouse
    function hookMouseSlot() {
        if(!background.doHookMouse())
            hookTimer.start();
    }
    function doHookMouse() {
        if(background.Window) {
            const screenArea = Common.findItem(Window.contentItem, "MouseEventListener");
            if(screenArea === null)
                return false;
            const screenGrid = Common.findItem(screenArea, "QQuickGridView");
            if(screenGrid === null)
                return false;
            console.log(screenGrid);
            background.mouseHooker = Qt.createQmlObject(`import QtQuick 2.12;
                    import com.github.catsout.wallpaperEngineKde 1.1
                    MouseGrabber {
                        z: -1
                        anchors.fill: parent
                    }
            `, screenGrid);
            return true;
       }
       return false;
    }

    WindowModel {
        id: windowModel
        screenGeometry: wallpaper.parent.screenGeometry
        activity: wallpaper.parent.activity
    }

    WallpaperListModel {
        id: wpListModel
        enabled: background.randomizeWallpaper
        workshopDirs: Common.getProjectDirs(background.steamlibrary)
        filterStr: background.filterStr

        function changeWallpaper(index) {
            if(this.model.count === 0) return;
            const model = this.model.get(index);
            wallpaper.configuration.WallpaperWorkShopId = model.workshopid;
            wallpaper.configuration.WallpaperFilePath = model.path + "/" + model.file;
            wallpaper.configuration.WallpaperType = model.type;
        }
    }
    Timer {
        id: randomizeTimer
        running: background.randomizeWallpaper
        interval: background.switchTimer * 1000 * 60
        repeat: true
        onTriggered: {
            const i = Math.round(Math.random() * wpListModel.model.count);
            wpListModel.changeWallpaper(i);
        }
    }


    // lauch pause time to avoid freezing
    Timer {
        id: lauchPauseTimer
        running: false
        repeat: false
        interval: 300
        onTriggered: {
            backendLoader.item.pause();
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
    // main  
    Loader { 
        id: backendLoader
        anchors.fill: parent
        Component.onCompleted: {
            if(background.hasLib) {
                backendLoader.loaded.connect(backendLoader.changeMouseTarget);
                background.mouseHookerChanged.connect(backendLoader.changeMouseTarget);
            }
        }
        function changeMouseTarget() {
           if(backendLoader.status == Loader.Ready && background.mouseHooker) {
                let re = backendLoader.item.getMouseTarget();
                if(!re)
                    re = null;
                background.mouseHooker.target = re;
           }
        }
    }
    
    function loadBackend() {
        let qmlsource = "";
        let properties = {
            wid: background.workshopid,
            type: background.type
        };

        // check source
        if(!background.source || background.source == "") {
            qmlsource = "backend/InfoShow.qml";
            properties["info"] = "Source is empty. The config may be broken.";
            backendLoader.setSource(qmlsource, properties);
            return;
        }
        // choose backend
        switch (background.type) {
            case 'video':
                if(background.useMpv && background.hasLib)
                    qmlsource = "backend/Mpv.qml";
                else qmlsource = "backend/QtMultimedia.qml";
                properties = {};
                break;
            case 'web':
                qmlsource = "backend/QtWebView.qml";
                properties = {};
                break;
            case 'scene':
                if(background.hasLib) {
                    qmlsource = "backend/Scene.qml";
                    properties = {"assets": background.getAssetsPath()};
                } else {
                    qmlsource = "backend/InfoShow.qml";
                    properties["info"] = "Plugin lib not found. To support scene, please compile and install it.";
                }
                break;
            default:
                qmlsource = "backend/InfoShow.qml";
                properties["info"] = "Not supported wallpaper type";
                break
        }
        console.log("load backend: "+qmlsource);
        backendLoader.setSource(qmlsource, properties);
        sourceCallback();
    }
   
    function autoPause() {background.ok
                    ? backendLoader.item.play()
                    : backendLoader.item.pause()
    }

    Component.onCompleted: {
        // load first backend
        loadBackend(); 

        // background signal connect
        background.typeChanged.connect(loadBackend);
        background.useMpvChanged.connect(loadBackend);
        background.sourceChanged.connect(sourceCallback);
        background.okChanged.connect(autoPause);

        lauchPauseTimer.start();
        randomizeTimer.start();
    }
}
