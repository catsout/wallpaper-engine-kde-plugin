import QtQuick 2.5
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2

import "js/utils.mjs" as Utils

Item {
    id: root
    property var workshopDirs
    property var globalConfigPath
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

    property var playlists: {}
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

    function loadPlaylists() {
        // reset playlists property
        root.playlists = {};
    
        return root._readfile(Common.urlNative(globalConfigPath)).then(value => {
            var jsonData = JSON.parse(value);

            // refreshing entries in the filter model is not thread safe, so we need to lock it
            var filterModel = Common.filterModel;
            return filterModel.lock.lock().then(() => {
                // remove playlists from the filterModel
                var selectedPlaylists = new Set();
                for(var i =0; i < filterModel.count; i++) {
                    var el = filterModel.get(i);
                    if(el.type == "playlist") {
                        if(el.def) { selectedPlaylists.add(el.key); }
                        filterModel.remove(i);
                        i--;
                    }
                }

                jsonData.steamuser.general.playlists.forEach(function(el) {
                    // we're going to be using paths to match wallpapers to playlists, but the paths in the config will start with a Windows-style drive letter
                    // so we need to convert them to file:// URLs. In addition it appears that the paths are truncated to 110 chars elsewhere so we will do the same
                    // so that they can match later
                    root.playlists[el.name] = new Set(el.items.map(el => "file://" + el.substring(2).replace(/\/[^\/]*$/, "").substring(0,110))); 
                    // add the playlist to the filter model preserving it's previous selection status
                    filterModel.append({type: "playlist", key: el.name, text: el.name, def: selectedPlaylists.has(el.name) ? 1 : 0});                    
                });
            })
            .then(() => { filterModel.lock.release() })
            .catch(() => { filterModel.lock.release() });
        }).catch(reason => console.error("PlaylistLoadError " + reason.lineNumber + " -- " + reason.type + reason.message));
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
            return filterToList(root.model, root.filterStr, this.model);
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
            return new Promise((resolve, reject) => {
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
                root.countNoFilter = this.model.length;
                root.modelRefreshed();
            });
        }
    }

    function refresh() {
        if(!root.enabled) return Promise.resolve(null);
        const p_list = [];

        return loadPlaylists().then(() => {
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
            return new Promise((resolve, reject) => {
                Promise.all(p_list).then(values => {
                    return this.loadFolderLists(values);
                }).then(() => {
                    resolve();
                }).catch(reason => {
                    console.error(reason)
                    resolve();
                });
            });
        });
    }

    function loadFolderLists(folders) {
        const proxyModel = []
        folders.forEach(folder => {
            if(!folder) return Promise.resolve();
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
        return new Promise((resolve, reject) => {
            const plist = []
            proxyModel.forEach((el) => {
                // as no allSettled, catch any error
                const p = root._readfile(Common.urlNative(Common.getWpModelProjectPath(el))).then(value => {                    
                        el.playlists = [];
                        root.loadItemFromJson(value, el);
                        Object.keys(root.playlists).forEach((key) => {
                            const value = root.playlists[key];
                            if(value.has(el.path)) {       
                                if(!el.playlists.includes(key))
                                    el.playlists.push(Object({key: key}));
                            }
                        });
                    }).catch(reason => console.error(reason));
                plist.push(p);
            });
            const path = this.folder;
            Promise.all(plist).then(value => {
                folderWorker.loadModel(path, proxyModel).then(() => resolve());
            }).catch(reason => {
                console.error(reason);
                resolve();
            });
        });

    }
    Component.onCompleted: {
        this.filterStrChanged.connect(function() {
            if(root.enabled) {
                return folderWorker.filterToList(root.model, root.filterStr, folderWorker.model)
            }
            return Promise.resolve();
        });
        this.sortModeChanged.connect(this.filterStrChanged);
        this.enabledChanged.connect(this.refresh.bind(this));

        const fc = this.readfile;
        this.readfileChanged.connect(function() {
            if(fc === root.readfile) return Promise.resolve();
            return root.refresh().then(() => { fc = root.readfile; });
        });
        return this.refresh();
    }

    // scan once
    Timer {
        running: true
        interval: 10000
        repeat: false   //run once
        onTriggered: {
            if(wpListModel.model.count === 0)
                return wpListModel.refresh();  //refresh to scan
            return Promise.resolve();
        }
    }

}
