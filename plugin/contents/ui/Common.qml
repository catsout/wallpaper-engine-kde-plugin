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
}
