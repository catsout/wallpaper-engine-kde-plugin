import QtQuick 2.5
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.2

import "js/utils.mjs" as Utils

Item {
    id: root
    property var workshopDirs
    property string filterStr: ""
    property int sortMode: Common.SortMode.Id
    property bool enabled: true

    property var initItemOp: null
    property var _initItemOp: Boolean(initItemOp) ? initItemOp : function(){ }
    property var readfile: null 
    property var _readfile: Boolean(readfile) ? readfile : function(){ return Promise.reject("read file func not available"); }

    signal modelStartSync
    signal modelRefreshed

    readonly property ListModel model: ListModel {
        function assignModel(index, value) {
            Object.assign(this.get(index), value);
            const workshopid = this.get(index).workshopid;
            new Promise((resolve, reject) => {
                const model = folderWorker.model;
                for(let i=0;i<model.length;i++) {
                    if(model[i].workshopid === workshopid) {
                        Object.assign(model[i], value);
                        resolve();
                    }
                }
                reject();
            });
        }
    }

    property int countNoFilter: 0

    property var folderModels: []

    function loadItemFromJson(text, el) {
        const project = Utils.parseJson(text);    
        if(project !== null) {
            if("title" in project)
                el.title = project.title;
            if("preview" in project && project.preview)
                el.preview = project.preview;
            if("file" in project)
                el.file = project.file;
            if("type" in project)
                el.type = project.type.toLowerCase();
            if("contentrating" in project)
                el.contentrating = project.contentrating;
            if("tags" in project) {
                el.tags = project.tags.map(el => Object({key: el}));
            }
        }
    }

    function genSortCmp(mode) {
        switch (mode) {
          case Common.SortMode.Modified:
            return function(a, b) {
                return -(a.modified - b.modified);
            }
          case Common.SortMode.Name:
            return function(a, b) {
                return a.title<b.title ? -1 : 1;
            }
          case Common.SortMode.Id:
          default:
            return function(a, b) {
                return a.workshopid<b.workshopid ? -1 : 1;
            };
        }
    }

    Item {
        id: folderWorker

        // array
        property var folderMapModel: new Map()
        property var model: []

        function loadModel(path, data) {
            this.folderMapModel.set(path, data);
            this.model = [];
            this.folderMapModel.forEach((value, key) => {
                this.model.push(...value);
            });
            filterToList(root.model, root.filterStr, this.model);
        }
        function filterToList(listModel, filterStr, data) {
            const filterValues = Common.filterModel.getValueArray(filterStr);
            const filterstr = Common.filterModel.map((el, index) => {
                    return {
                        type: el.type,
                        key: el.key,
                        value: filterValues[index]
                    };
                });
            root.modelStartSync();
            new Promise((resolve, reject) => {
                const filter = Common.filterModel.genFilter(filterstr);
                const model = listModel;
                data.sort(genSortCmp(sortMode));
                model.clear();
                data.forEach(function(el) {
                    if(filter(el))
                        model.append(el);
                });
                resolve();
            }).then(() => {
                console.error(`filtered, filter: ${root.filterStr}, from ${this.model.length} to ${root.model.count}`);
                root.countNoFilter = this.model.length;
                root.modelRefreshed();
            });
        }
    }

    function refresh() {
        if(!root.enabled) return;
        const p_list = [];
        this.workshopDirs.forEach(el => {
            const dirs = (Array.isArray(el) ? el : [el]).map(Common.urlNative);
            p_list.push(pyext.get_folder_list(
                dirs[0],
                { only_dir: true, fallbacks: dirs.slice(1) }
            ).then(res => {
                if(!res) console.error(`folder not found: ${dirs[0]}`);
                return res;
            }).catch(reason => console.error(reason)));
        });
        new Promise((resolve, reject) => {
            Promise.all(p_list).then(values => {
                this.loadFolderLists(values);
            }).catch(reason => console.error(reason));
            resolve();
        });

    }

    function loadFolderLists(folders) {
        const proxyModel = []
        folders.forEach(folder => {
            if(!folder) return;
            // seems qml's "for" is a function
            const folder_dir = folder.folder;
            folder.items.forEach(el => {
                const v = Object.assign({}, Common.wpitem_template);
                v.workshopid = el.name;
                // use qurl to convert to file://
                v.path = Qt.resolvedUrl(folder_dir + '/' + el.name).toString();
                v.modified = el.mtime;
                root._initItemOp(v);
                proxyModel.push(v);
            });
            //if(proxyModel) console.error(`show the first: ${proxyModel[0].path}`)
        });
        new Promise((resolve, reject) => {
            const plist = []
            proxyModel.forEach((el) => {
                // as no allSettled, catch any error
                const p = root._readfile(Common.urlNative(Common.getWpModelProjectPath(el))).then(value => {
                        root.loadItemFromJson(value, el);
                    }).catch(reason => console.error(reason));
                plist.push(p);
            });
            const path = this.folder;
            Promise.all(plist).then(value => {
                folderWorker.loadModel(path, proxyModel);
            }).catch(reason => console.error(reason));
            resolve();
            /*
            const msg = {
                action: "loadFolder", 
                data: proxyModel,
                path: this.folder
            };
            sendMessage(msg);
            */
        });

    }
    Component.onCompleted: {
        this.filterStrChanged.connect(function() {
            if(root.enabled) {
                folderWorker.filterToList(root.model, root.filterStr, folderWorker.model)
            }
        });
        this.sortModeChanged.connect(this.filterStrChanged);
        this.enabledChanged.connect(this.refresh.bind(this));

        const fc = this.readfile;
        this.readfileChanged.connect(function() {
            if(fc === root.readfile) return;
            root.refresh();
            fc = root.readfile;
        });
        this.refresh();
    }

    // scan once
    Timer {
        running: true
        interval: 10000
        repeat: false   //run once
        onTriggered: {
            if(wpListModel.model.count === 0)
                wpListModel.refresh();  //refresh to scan
        }
    }

}
