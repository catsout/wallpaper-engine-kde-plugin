import QtQuick 2.12
import com.github.catsout.wallpaperEngineKde 1.2
import QtQuick.Window 2.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid

WallpaperItem {
Rectangle {
    id: background
    anchors.fill: parent
    color: wallpaper.configuration.BackgroundColor
    
    property string steamlibrary: Qt.resolvedUrl(wallpaper.configuration.SteamLibraryPath).toString()
    property string source: Qt.resolvedUrl(wallpaper.configuration.WallpaperSource).toString()

    property string filterStr: wallpaper.configuration.FilterStr

    property int    videoBackend: wallpaper.configuration.VideoBackend
    property int    switchTimer: wallpaper.configuration.SwitchTimer
    property int    fps: wallpaper.configuration.Fps

    property bool   randomizeWallpaper: wallpaper.configuration.RandomizeWallpaper
    property bool   noRandomWhilePaused: wallpaper.configuration.NoRandomWhilePaused
    property bool   mouseInput: wallpaper.configuration.MouseInput
    property bool   mpvStats: wallpaper.configuration.MpvStats

    property bool   pauseOnBatPower: wallpaper.configuration.PauseOnBatPower
    property int    pauseBatPercent: wallpaper.configuration.PauseBatPercent

    
    property var curOpt: ({})
    property string workshopid: {
        const wid = wallpaper.configuration.WallpaperWorkShopId;
        pyext.read_wallpaper_config(wid).then((res) => this.curOpt = res);
        return wid;
    }
    function get_opt_value(key, def) {
        if(curOpt.hasOwnProperty(key))
            return curOpt[key];
        return def;
    }
    property int    displayMode: get_opt_value('display_mode', wallpaper.configuration.DisplayMode)
    property bool   mute: get_opt_value('mute_audio', wallpaper.configuration.MuteAudio)
    property int    volume: get_opt_value('volume', wallpaper.configuration.Volume)
    property real    speed: get_opt_value('speed', wallpaper.configuration.Speed)

    property bool   perOptChanged: wallpaper.configuration.PerOptChanged
    onPerOptChangedChanged: {
        pyext.read_wallpaper_config(workshopid).then((res) => this.curOpt = res);
    }

    // auto pause
    property bool   ok: !windowModel.reqPause && !powerSource.reqPause

    // detect TTY switch and pause wallpaper(s)
    TTYSwitchMonitor {
        id: ttyMonitor
        onTtySwitch: {
            if (sleep) {
                console.log("Preparing for sleep (possibly a VT switch)");
                this.pause();
            } else {
                console.log("Waking up (VT switch back)");
                this.play();
            }
        }
    }

    property string nowBackend: ""

    property var mouseHooker
    property bool hasLib: Common.checklib_wallpaper(background)

    property var customConf: Common.loadCustomConf(wallpaper.configuration.CustomConf)

    property string wallpaperPath
    property string wallpaperType

    signal sig_backendFirstFrame(string backname)
    function onBackendFirstFrame(backname) {
        console.error(`backend ${backname} first frame`);
        if (wallpaper.hasOwnProperty('accentColor'))
            wallpaper.accentColorChanged();
    }

    Component.onDestruction: {
        if(mouseHooker) {
            mouseHooker.destroy();
        }
    }

    function applySource() {
        const { path, type } = Common.unpackWallpaperSource(source);
        const path_changed = background.wallpaperPath !== path;
        const type_changed = background.wallpaperType !== type;
        const is_infobackend = background.nowBackend === "InfoShow";

        if(type_changed) wallpaperType = type;
        if(path_changed) wallpaperPath = path;

        if(type_changed || is_infobackend || !source) {
            loadBackend();
        } else if(path_changed) {
            backendLoader.item.source = path;
        }

        sourceCallback();
    }

    function getWorkshopIDPath() {
        return Common.getWorkshopDir(this.steamlibrary) + `/${this.workshopid}`;
    }

    onMouseInputChanged: {
        if(this.mouseInput) {
            hookTimer.start();
        }
        else if(this.mouseHooker) {
            this.mouseHooker.target = null;
            this.mouseHooker.destroy;
            this.mouseHooker = null;
        }
    }

    Timer {
        id: hookTimer
        running: true
        repeat: false
        interval: 2000
        property int tryTimes: 0
        onTriggered: {
            tryTimes++;
            if(tryTimes >= 10 || !background.hasLib || !background.mouseInput) return;
            if(background.mouseHooker) return;
            background.hookMouse();
        }
        Component.onCompleted: {
            background.hookMouse.connect(background.hookMouseSlot);
        }
    }
    signal hookMouse
    function hookMouseSlot() {
        if(!background.doHookMouse()) {
            hookTimer.start();
        } else {
            hookTimer.tryTimes = 0;
        }
    }
    function doHookMouse() {
        if(background.Window) {
            const screenArea = Common.findItem(Window.contentItem, "MouseEventListener");
            if(screenArea === null)
                return false;
            const screenGrid = Common.findItem(screenArea, "QQuickGridView");
            if(screenGrid === null)
                return false;
            console.error(screenGrid);
            if(background.mouseHooker) background.mouseHooker.destroy();
            background.mouseHooker = Qt.createQmlObject(`import QtQuick 2.12;
                    import com.github.catsout.wallpaperEngineKde 1.2
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
        filterByScreen: wallpaper.configuration.PauseFilterByScreen
        modePlay: wallpaper.configuration.PauseMode
        resumeTime: wallpaper.configuration.ResumeTime
    }

    PowerSource {
        id: powerSource
        readonly property bool reqPause: {
            (background.pauseOnBatPower && (st_battery_state == 'NoCharge' || st_battery_state == 'Discharging')) ||
            (background.pauseBatPercent !== 0 && st_battery_has && st_battery_percent < background.pauseBatPercent)
        }
    }

    Pyext {
        id: pyext
    }
    WallpaperListModel {
        id: wpListModel
        enabled: background.randomizeWallpaper
        workshopDirs: Common.getProjectDirs(background.steamlibrary)
        globalConfigPath: Common.getGlobalConfigPath(background.steamlibrary)
        filterStr: background.filterStr
        initItemOp: (item) => {
            if(!background.customConf) return;
            item.favor = background.customConf.favor.has(item.workshopid);
        }
        readfile: pyext.readfile

        function changeWallpaper(index) {
            if(this.model.count === 0) return;
            const model = this.model.get(index);
            wallpaper.configuration.WallpaperWorkShopId = model.workshopid;
            wallpaper.configuration.WallpaperSource = Common.packWallpaperSource(model);
        }
    }
    Timer {
        id: randomizeTimer
        running: background.randomizeWallpaper
        interval: background.switchTimer * 1000 * 60
        repeat: true
        onTriggered: {
            if(!(background.noRandomWhilePaused && !background.ok)) {
                const i = Math.round(Math.random() * wpListModel.model.count);
                wpListModel.changeWallpaper(i);
            }
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
    Item { 
        id: backendLoader
        anchors.fill: parent
        property var item: null

        signal loaded

        Component.onCompleted: {
            if(background.hasLib) {
                this.loaded.connect(this.changeMouseTarget);
                background.mouseHookerChanged.connect(this.changeMouseTarget);
            }
        }
        Component.onDestruction: {
            if(this.item) this.item.destroy();
        }
        function load(url, properties) {
            const com = Qt.createComponent(url);
            if(com.status === Component.Ready) {
                if(this.item) this.item.destroy(100);
                this.item = null;
                try {
                    this.item = com.createObject(this, properties);
                } catch(e) {
                    this.loadInfoShow(e);
                    return;
                }
                this.loaded();
            } else if(com.status == Component.Error) {
                this.loadInfoShow(com.errorString());
            }
        }
        function loadInfoShow(info) {
            this.load("backend/InfoShow.qml", {
                wid: background.workshopid,
                type: background.wallpaperType,
                info: info
            });
        }
        function changeMouseTarget() {
           if(backendLoader.item && background.mouseHooker) {
                let re = backendLoader.item.getMouseTarget();
                if(!re)
                    re = null;
                background.mouseHooker.target = re;
           }
        }
    }

    function loadBackend() {
        let qmlsource = "";
        let properties = {};

    
        // check source
        if(!background.source) {
            backendLoader.loadInfoShow("Source is empty. The config may be broken.");
            return;
        }
        // choose backend
        switch (background.wallpaperType) {
            case 'video':
                if(background.videoBackend == Common.VideoBackend.Mpv && background.hasLib)
                    qmlsource = "backend/Mpv.qml";
                else qmlsource = "backend/QtMultimedia.qml";
                properties = {};
                break;
            case 'web':
                qmlsource = "backend/QtWebView.qml";
                properties = {readfile: pyext.readfile};
                break;
            case 'scene':
                if(background.hasLib) {
                    qmlsource = "backend/Scene.qml";
                    properties = {"assets": Common.getAssetsPath(steamlibrary)};
                } else {
                    backendLoader.loadInfoShow("Plugin lib not found. To support scene, please compile and install it.");
                    return; 
                }
                break;
            default:
                backendLoader.loadInfoShow("Not supported wallpaper type");
                return; 
        }
        properties['source'] = background.wallpaperPath;
        console.error("load backend: "+qmlsource);
        backendLoader.load(qmlsource, properties);
        sourceCallback();
    }
   
    function autoPause() {
        background.ok
            ? backendLoader.item.play()
            : backendLoader.item.pause();
    }

    Component.onCompleted: {
        // load first backend
        applySource();

        // background signal connect
        background.videoBackendChanged.connect(loadBackend);
        background.okChanged.connect(autoPause);
        background.sourceChanged.connect(applySource);

        lauchPauseTimer.start();
        randomizeTimer.start();
    }
}
}
