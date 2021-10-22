import QtQuick 2.5
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.2

import Qt.labs.folderlistmodel 2.11
import "utils.mjs" as Utils

Item {
    id: root
    property var workshopDirs
    property string filterStr
    property bool enabled: true

    property var initItemOp: null
    property var _initItemOp: initItemOp ?? function(){ }
    property var readfile: null 
    property var _readfile: readfile ?? function(){ return Promise.reject("read file func not available"); }

    signal modelStartSync
    signal modelRefreshed

    readonly property var itemTemplate: ({
        workshopid: "",
        path: "", // need convert to qurl
        loaded: false,
        title: "unknown",
        preview: "",
        type: "unknown",
        contentrating: "Everyone",
        tags: [],
        favor: false
    })

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
            if("tags" in project)
                el.tags = project.tags;
        }
    }


    WorkerScript {
        id: folderWorker
        source: "folderWorker.mjs"

        // array
        property var folderMapModel: new Map()
        property var model: []

        onMessage: {
            if(messageObject.reply == "loadFolder") {
                // loadModel 
            } else if(messageObject.reply == "filter") {
                console.log(`filtered, filter: ${root.filterStr}, from ${this.model.length} to ${root.model.count}`);
                root.countNoFilter = this.model.length;
                root.modelRefreshed();
            }
        }
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

            const msg = {
                action: "filter", 
                data: data,
                model: listModel,
                filters: Common.filterModel.map((el, index) => {
                    return {
                        type: el.type,
                        key: el.key,
                        value: filterValues[index]
                    };
                })
            };

            this.sendMessage(msg);
            root.modelStartSync();
        }

        Component.onCompleted: {
            root.filterStrChanged.connect(function() {
                if(this.enabled) {
                    folderWorker.filterToList(this.model, this.filterStr, folderWorker.model)
                }
            }.bind(root));
        }
    }
    function refresh() {
        if(!root.enabled) return;
        this.folderModels.forEach(el => el.destroy());
        this.folderModels = [];
        this.workshopDirs.forEach(el => {
            this.folderModels.push(folderCom.createObject(this, {
                folder: el,
                requirFolder: el
            }));
        });
    }
    Component.onCompleted: {
        this.refresh();
    }
    Component {
        id: folderCom
        FolderListModel {
            property url requirFolder
            onStatusChanged: {
                if(this.folder != requirFolder) { 
                    console.log(`require: ${requirFolder}, but get: ${this.folder}`);
                    return;
                }
                if (root.enabled && this.status === FolderListModel.Ready) {
                    console.log(`scan folder: ${this.folder}, found ${this.count} subdir`);
                    const proxyModel = []
                    const sendMessage = folderWorker.sendMessage.bind(folderWorker);
                    new Promise((resolve, reject) => {
                        // seems qml's "for" is a function
                        const count = this.count;
                        const get = this.get.bind(this);
                        for(let i=0;i < count;i++) {
                            const v = Object.assign({}, root.itemTemplate);
                            v.workshopid = get(i,"fileName");
                            // use qurl to convert to file://
                            v.path = Qt.resolvedUrl(get(i,"filePath")).toString();
                            root._initItemOp(v);
                            proxyModel.push(v);
                            if(i === 0) {
                                console.log(`show the first: ${v.path}`)
                            }
                        }
                        resolve();
                    }).then((value) => {
                        const plist = []
                        proxyModel.forEach((el) => {
                            // as no allSettled, catch any error
                            const p = root._readfile(Common.urlNative(el.path)+"/project.json").then(value => {
                                    loadItemFromJson(value, el);
                                }).catch(reason => console.error(reason));
                            plist.push(p);
                        });
                        const path = this.folder;
                        Promise.all(plist).then(value => {
                            folderWorker.loadModel(path, proxyModel);
                        }).catch(reason => console.error(reason));
                        /*
                        const msg = {
                            action: "loadFolder", 
                            data: proxyModel,
                            path: this.folder
                        };
                        sendMessage(msg);
                        */
                    }).catch(reason => console.error(reason));
                }
            }
        }
    }
}
