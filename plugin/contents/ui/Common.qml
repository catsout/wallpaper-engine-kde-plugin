pragma Singleton
import QtQuick 2.0
import "utils.mjs" as Utils

QtObject {
    enum PauseMode {
        Never,
        Any,
        Max
    }
    enum DisplayMode {
        Aspect,
        Crop,
        Scale
    }
    property string wpenginePath: "/steamapps/workshop/content/431960"

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


        function map(func) {
            const arr = [];
            for(let i=0;i<this.count;i++) arr.push(func(this.get(i), i));
            return arr;
        }

        function getValueArray(filterStr) {
            // not empty
            const result = strToIntArray(filterStr);
            if(result.length < this.count) {
                return filterModel.map((el) => el.def);
            } else {
                return result;
            }
        }
    }

/*
    // const
    property var filterModelComp: Component { 
        ListModel {
            ListElement { text: "scene"; type:"type"; key:"scene"; value:1 }
            ListElement { text: "web"; type:"type"; key:"web"; value:1 }
            ListElement { text: "video"; type:"type"; key:"video"; value:1 }
            function map(func) {
                const arr = [];
                for(let i=0;i<this.count;i++) arr.push(func(this.get(i), i));
                return arr;
            }
        }
    }
*/

    function getWorkshopDir(steamLibraryPath) {
        return steamLibraryPath + "/steamapps/workshop/content/431960";
    }
    function getDefProjectsDir(steamLibraryPath) {
        return steamLibraryPath + "/steamapps/common/wallpaper_engine/projects/defaultprojects";
    }
    function getMyProjectsDir(steamLibraryPath) {
        return steamLibraryPath + "/steamapps/common/wallpaper_engine/projects/myprojects"
    }
    function getProjectDirs(steamLibraryPath) {
        return [
            getWorkshopDir(steamLibraryPath),
            getDefProjectsDir(steamLibraryPath),
            getMyProjectsDir(steamLibraryPath)
        ];
    }
    function getAssetsPath(steamLibrary) {
        return steamLibrary + "/steamapps/common/wallpaper_engine/assets";
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
            console.log("---check lib '"+libName+"' failed---");
            console.log(error);
        }
        if(create != null){
            ok = true;
            create.destroy(1000);
        }
        return ok;
    }

    function checklib_wallpaper(parentItem) {
        return checklib('com.github.catsout.wallpaperEngineKde 1.1', parentItem);
    }

    function checklib_folderlist(parentItem) {
        return checklib('Qt.labs.folderlistmodel 2.11', parentItem)
    }

    function trimCharR(str, c) {
        let pos = 0;
        while(str.slice(pos - 1, str.length + pos) === c) {
            pos -= 1;
        }
        return str.slice(0, str.length + pos);
    }

    function cbCurrentValue(combo) {
        return combo.model[combo.currentIndex].value;
    }

    function cbIndexOfValue(combo, value) {
        const model = combo.model;
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

    function strToIntArray(str) {
        return [...str].map((e) => e.charCodeAt(0) - '0'.charCodeAt(0));
    }
    function intArrayToStr(arr) {
        return arr.reduce((acc, e) => acc + e.toString(), "");
    }
    function clearArray(arr) {
        arr.length = 0; 
    }

    function listProperty(item) {
        for (var p in item)
            console.log(p + ": " + item[p]);
    }

}
