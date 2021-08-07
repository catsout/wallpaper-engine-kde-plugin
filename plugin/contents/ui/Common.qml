pragma Singleton
import QtQuick 2.0

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
        ListElement { text: "scene"; type:"type"; key:"scene"; }
        ListElement { text: "web"; type:"type"; key:"web"; }
        ListElement { text: "video"; type:"type"; key:"video"; }
        function map(func) {
            let arr = [];
            for(let i=0;i<this.count;i++) arr.push(func(this.get(i), i));
            return arr;
        }

        function getValueArray(filterStr) {
            // not empty
            const result = strToIntArray(filterStr);
            if(result.length < this.count) {
                return this.map((el) => 1);
            } else {
                return result;
            }
        }
    }

    // const
    property var filterModelComp: Component { 
        ListModel {
            ListElement { text: "scene"; type:"type"; key:"scene"; value:1 }
            ListElement { text: "web"; type:"type"; key:"web"; value:1 }
            ListElement { text: "video"; type:"type"; key:"video"; value:1 }
            function map(func) {
                let arr = [];
                for(let i=0;i<this.count;i++) arr.push(func(this.get(i), i));
                return arr;
            }
        }
    }

    function checklib(libName, parentItem) {
        let ok = false;
        let create = null;
         try {
            create = Qt.createQmlObject(
            'import '+ libName +';import QtQml 2.2; QtObject{}',
            parentItem);

        } catch (error) {}
        if(create != null){
            ok = true;
            create.destroy(1000);
        }
        return ok;
    }

    function checklib_wallpaper(parentItem) {
        return checklib('com.github.catsout.wallpaperEngineKde 1.0', parentItem);
    }

    function checklib_folderlist(parentItem) {
        return checklib('Qt.labs.folderlistmodel 2.11', parentItem)
    }

    function readTextFile(fileUrl, callback) {
        let xhr = new XMLHttpRequest;
        xhr.open("GET", fileUrl);
        xhr.onreadystatechange = function () {
            if(xhr.readyState === XMLHttpRequest.DONE){
                let response = xhr.responseText;
                if(typeof(response) === "string")
                    callback(response);
                else
                    callback("");
            }
        }
        xhr.send();
    }

    function trimCharR(str, c) {
        let pos = 0;
        while(str.slice(pos - 1, str.length + pos) === c) {
            pos -= 1;
        }
        return str.slice(0, str.length + pos);
    }

    function parseJson(str) {
        let obj_j;
        try {
            obj_j = JSON.parse(str);
        } catch (e) {
            if (e instanceof SyntaxError) {
                console.log(e.message);
                obj_j = null;
            } else {
              throw e;  // re-throw the error unchanged
            }
        } 
        return obj_j;
    }

    function cbCurrentValue(combo) {
        return combo.model[combo.currentIndex].value;
    }

    function cbIndexOfValue(combo, value) {
        let model = combo.model;
        for(let i=0;i < model.length;i++) {
            if(model[i].value == value) {
                return i;
            }
        }
        return -1;
    }

    function findItem(item, typename) {
        let name = item.toString();
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
        let timer = Qt.createQmlObject(`import QtQuick 2.0; Timer {
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
}
