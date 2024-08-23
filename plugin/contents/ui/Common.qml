pragma Singleton
import QtQuick 2.0

import "js/utils.mjs" as Utils

QtObject {
    enum PauseMode {
        Never,
        Any,
        Max,
        Focus,
        FocusOrMax,
        FullScreen
    }
    enum DisplayMode {
        Aspect,
        Crop,
        Scale
    }
    enum SortMode {
        Id,
        Name,
        Modified
    }
    enum VideoBackend {
        QtMultimedia,
        Mpv
    }

    readonly property string version: '0.5.5'

    readonly property string repo_url: 'https://github.com/catsout/wallpaper-engine-kde-plugin'

    readonly property var wpitem_template: ({
        workshopid: "",
        path: "", // need convert to qurl
        loaded: false,
        title: "unknown",
        preview: "",
        type: "unknown",
        contentrating: "Everyone",
        tags: [],
        favor: false,
        playlists: []
    })

    function wpitemFromQtObject(qobj) {
        const v = Object.assign({}, Common.wpitem_template);
        for(const prop in wpitem_template) {
            v[prop] = qobj[prop];
        }
        return v;
    }

    property var filterModel: ListModel {
        ListElement { text: "Favorite";     type:"favor";         key:"favor";         def: 0}
        ListElement { text: "TYPE";         type:"_nocheck";      key:"";              def: 1}
        ListElement { text: "Scene";        type:"type";          key:"scene";         def: 1}
        ListElement { text: "Web";          type:"type";          key:"web";           def: 1}
        ListElement { text: "Video";        type:"type";          key:"video";         def: 1}
        ListElement { text: "AGE";          type:"_nocheck";      key:"";              def: 1}
        ListElement { text: "Everyone";     type:"contentrating"; key:"Everyone";      def: 1}
        ListElement { text: "Questionable"; type:"contentrating"; key:"Questionable";  def: 1}
        ListElement { text: "Mature";       type:"contentrating"; key:"Mature";        def: 0}
        ListElement { text: "GENRE";        type:"_nocheck";      key:"";              def: 1}
        ListElement { text: "Abstract";     type:"tags";          key:"Abstract";      def: 1}
        ListElement { text: "Animal";       type:"tags";          key:"Animal";        def: 1}
        ListElement { text: "Anime";        type:"tags";          key:"Anime";         def: 1}
        ListElement { text: "Cartoon";      type:"tags";          key:"Cartoon";       def: 1}
        ListElement { text: "CGI";          type:"tags";          key:"CGI";           def: 1}
        ListElement { text: "Cyberpunk";    type:"tags";          key:"Cyberpunk";     def: 1}
        ListElement { text: "Fantasy";      type:"tags";          key:"Fantasy";       def: 1}
        ListElement { text: "Game";         type:"tags";          key:"Game";          def: 1}
        ListElement { text: "Girls";        type:"tags";          key:"Girls";         def: 1}
        ListElement { text: "Guys";         type:"tags";          key:"Guys";          def: 1}
        ListElement { text: "Landscape";    type:"tags";          key:"Landscape";     def: 1}
        ListElement { text: "Medieval";     type:"tags";          key:"Medieval";      def: 1}
        ListElement { text: "Memes";        type:"tags";          key:"Memes";         def: 1}
        ListElement { text: "MMD";          type:"tags";          key:"MMD";           def: 1}
        ListElement { text: "Music";        type:"tags";          key:"Music";         def: 1}
        ListElement { text: "Nature";       type:"tags";          key:"Nature";        def: 1}
        ListElement { text: "Pixel art";    type:"tags";          key:"Pixel art";     def: 1}
        ListElement { text: "Relaxing";     type:"tags";          key:"Relaxing";      def: 1}
        ListElement { text: "Retro";        type:"tags";          key:"Retro";         def: 1}
        ListElement { text: "Sci-Fi";       type:"tags";          key:"Sci-Fi";        def: 1}
        ListElement { text: "Sports";       type:"tags";          key:"Sports";        def: 1}
        ListElement { text: "Technology";   type:"tags";          key:"Technology";    def: 1}
        ListElement { text: "Television";   type:"tags";          key:"Television";    def: 1}
        ListElement { text: "Vehicle";      type:"tags";          key:"Vehicle";       def: 1}
        ListElement { text: "Unspecified";  type:"tags";          key:"Unspecified";   def: 1}
        ListElement { text: "PLAYLIST";     type:"_nocheck";      key:"";              def: 1}

        // need to be able to lock the filterModel because when settings are being applied to multiple screens simultaneously,
        // the filtermodel can be modified by multiple threads at the same time. This can lead to having excess playlist entries, which will cause
        // the filter value to be rejected by the getValueArray function for being shorter than the numebr of filter elements, causing it to drop 
        // the filter value and use the default value instead.This is particularly problematic when using the randomizer timer across multiple screens.
        property var lock: ({
            locked : false,
            lock_resolvers: [],
            lock: function() {
                return new Promise((resolve, reject) => {
                    if(!this.locked) {
                        this.locked = true;
                        resolve();
                    } else {
                        this.lock_resolvers.push(resolve);
                    }
                });            
            },
            release: function() {
                if(this.lock_resolvers.length > 0) {
                    this.lock_resolvers.shift()();
                } else {
                    this.locked = false;
                }
            }
        })

        function map(func) {
            const arr = [];
            for(let i=0;i<this.count;i++) arr.push(func(this.get(i), i));
            return arr;
        }

        function getValueArray(filterStr) {
            // not empty
            const result = Utils.strToIntArray(filterStr);
            if(result.length < this.count) {
                return filterModel.map((el) => el.def);
            } else {
                return result;
            }
        }

        function genFilter(filters) {
            const typeF = {};
            const noTags = new Set();
            const playlists = new Set();
            let onlyFavor = false;
            filters.forEach((el) => {
                if(el.type === "type") 
                    typeF[el.key] = el.value;
                else if(el.type === "contentrating")
                    typeF[el.key] = el.value;
                else if(el.type === "favor") 
                    onlyFavor = el.value;
                else if(el.type === "tags") {
                    if(!el.value) noTags.add(el.key)
                }
                else if(el.type === "playlist") {
                    if(el.value) {
                        playlists.add(el.key);
                    }                    
                }
            });

            const checkType = (el) => Boolean(typeF[el.type]);
            const checkContentrating = (el) => Boolean(typeF[el.contentrating]);
            const checkFavor = (el) => onlyFavor?el.favor:true;
            const checkNoTags = (el) => {
                for(let i=0;i < el.tags.length;i++) {
                    if(noTags.has(el.tags[i].key)) {
                        return false;
                    }
                }
                return true;
            }
            const checkPlaylists = (el) => {
                if(playlists.size === 0) return true;
                for(let i=0; i < el.playlists.length;i++) {
                    if(playlists.has(el.playlists[i].key)) {
                        return true;
                    }
                }
                return false;
            }
            return (el) => {
                return checkType(el) && checkFavor(el) && checkContentrating(el) && checkNoTags(el) && checkPlaylists(el);
            }
        }
    }
    
    readonly property var regex_workshop_online: new RegExp('^[0-9]+$', 'g');
    readonly property var regex_path_check: new RegExp('^file://.+?(431960/[0-9]+$|wallpaper_engine/projects/[a-z]+/.+)', 'g');
    readonly property var regex_source: new RegExp('^(.+)\\+([a-z]+)$', '');

    function getWorkshopDir(steamLibraryPath) {
        return steamLibraryPath + "/steamapps/workshop/content/431960";
    }
    function getWorkshopDirs(steamLibraryPath) {
        return [
            "/steamapps/workshop/content/431960",
            "/Steamapps/Workshop/content/431960",
            "/Steamapps/Workshop/Content/431960",
            "/steamapps/Workshop/Content/431960",
        ].map(el => steamLibraryPath + el);
    }
    function getDefProjectsDir(steamLibraryPath) {
        return steamLibraryPath + "/steamapps/common/wallpaper_engine/projects/defaultprojects";
    }
    function getMyProjectsDir(steamLibraryPath) {
        return steamLibraryPath + "/steamapps/common/wallpaper_engine/projects/myprojects"
    }
    function getProjectDirs(steamLibraryPath) {
        return [
            getWorkshopDirs(steamLibraryPath),
            getDefProjectsDir(steamLibraryPath),
            getMyProjectsDir(steamLibraryPath)
        ];
    }
    function getAssetsPath(steamLibrary) {
        return steamLibrary + "/steamapps/common/wallpaper_engine/assets";
    }
    function getGlobalConfigPath(steamLibrary) {
        return steamLibrary + "/steamapps/common/wallpaper_engine/config.json";
    }
    function getWorkshopUrl(workshopid) {
        return "https://steamcommunity.com/sharedfiles/filedetails/?id=" + workshopid;
    }
    
    
    // wallpaper list modle
    function getWpModelPreviewSource(model) {
        return model.preview ? `${model.path}/${model.preview}` : '';
    }
    function getWpModelFileSource(model) {
        return model.path ? `${model.path}/${model.file}+${model.type}` : '';
    }
    function getWpModelProjectPath(model) {
        return model.path ? `${model.path}/project.json` : '';
    }

    function packWallpaperSource(model) {
        return model.path ? `${model.path}/${model.file}+${model.type}` : '';
    }
    function unpackWallpaperSource(source) {
        const match = source.match(regex_source);
        return {
            path: match ? match[1] : '',
            type: match ? match[2] : ''
        };
    }

    function loadCustomConf(data) {
        const conf = {
            favor: new Set()
        };
        try {
            const jsonStr = Qt.atob(data);
            Object.assign(conf, Utils.parseJson(jsonStr));
            conf.favor = new Set(conf.favor);
            return conf;
        } catch(e) { 
            console.error(e);
            return conf; 
        }
    }
    function prepareCustomConf(conf) {
        function setTojson(key, value) {
            if (value instanceof Set) {
                return [...value];
            }
            return value;
        }
        return Qt.btoa(JSON.stringify(conf, setTojson));
    }

    function checklib(libName, parentItem) {
        let ok = false;
        let create = null;
        try {
            create = Qt.createQmlObject(
            'import '+ libName +';import QtQml 2.2; QtObject{}',
            parentItem);

        } catch (error) {
            console.error("---check lib '"+libName+"' failed---");
            console.error(error);
        }
        if(create){
            ok = true;
            create.destroy(1000);
        }
        return ok;
    }

    function checklib_wallpaper(parentItem) {
        return checklib('com.github.catsout.wallpaperEngineKde 1.2', parentItem);
    }

    function checklib_folderlist(parentItem) {
        return checklib('Qt.labs.folderlistmodel 2.11', parentItem)
    }

    function checklib_websockets(parentItem) {
        return checklib('QtWebSockets 1.0', parentItem)
    }

    function checklib_webchannel(parentItem) {
        return checklib('QtWebChannel 1.10', parentItem)
    }


    function cbCurrentValue(combo) {
        return combo.model[combo.currentIndex].value;
    }
    function cbValueOfIndex(combo, index) {
        return combo.model[index].value;
    }
    function cbIndexOfValue(combo, value) {
        return modelIndexOfValue(combo.model,value);
    }
    function modelIndexOfValue(model, value) {
        for(let i=0;i < model.length;i++) {
            if(model[i].value == value) {
                return i;
            }
        }
        return -1;
    }

    function findItem(item, typename) {
        const name = item.toString();
        if(name.substring(0, typename.length) == typename) {
            return item;
        }
        for (let i = 0; i < item.children.length; i++) {
            let re = findItem(item.children[i], typename); 
            if(re !== null) {
                return re;
            }
        }
        return null;
    }
    function genItemListStr(item, indent0, tostr) {
        function gen(item, indent) {
            let res = `${indent}${tostr(item)}\n`
            for (let i = 0; i < item.children.length; i++) {
                res += gen(item.children[i], indent + indent0); 
            }
            return res;
        }
        return gen(item, "");
    }
    function createVolumeFade(qobj, volume, changePlayerVolum) {
        const timer = Qt.createQmlObject(`import QtQuick 2.0; Timer {
            property real volume
            property real volumeFade: 0.0
        }`, qobj);
        timer.interval = 300;
        timer.repeat = true;
        timer.volume = volume;
        timer.triggered.connect(function() {
            if(this.volumeFade >= this.volume) {
                this.volumeFade = this.volume;
                timer.stop();
            } else
                this.volumeFade += 5;
        }.bind(timer));
        timer.onVolumeChanged.connect(function() {
            if(!this.running) this.volumeFade = this.volume;
        }.bind(timer));
        timer.onVolumeFadeChanged.connect(function() {
            changePlayerVolum(this.volumeFade);
        }.bind(timer));
        return {
            start: () => {
                timer.start();
            },
            stop: () => {
                timer.volumeFade = 0.0;
                timer.stop();
            }
        };
    }

    function listProperty(item) {
        for (var p in item)
            console.error(p, ":", item[p]);
    }
    
    function urlNative(url) {
        const str = url.toString();
        if(str.startsWith('file://')) {
            return str.slice(7);
        } else if(str.startsWith('file:')) {
            return str.slice(5);
        }
        return str;
    }
}
