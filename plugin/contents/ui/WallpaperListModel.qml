import QtQuick 2.5
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.2

import Qt.labs.folderlistmodel 2.11

Item {
    id: wpItem
    property url workshopDir
    property string filterStr
    property bool enabled: true

    signal modelRefreshed

    readonly property var itemTemplate: ({
        workshopid: "",
        path: "", // need convert to qurl
        loaded: false,
        title: "unknown",
        preview: "unknown",
        type: "unknown"
    })

    readonly property ListModel model: ListModel {}

    WorkerScript {
        id: folderWorker
        source: "folderWorker.mjs"

        // array
        property var proxyModel: []

        onMessage: {
            if(messageObject.reply == "loadFolder") {
                proxyModel = messageObject.data;
                filterToList(wpItem.model, wpItem.filterStr);
            } else if(messageObject.reply == "filter") {
                wpItem.modelRefreshed()
            }
        }
        function filterToList(listModel, filterStr) {
            const filterValues = Common.filterModel.getValueArray(filterStr);

            const msg = {
                action: "filter", 
                data: this.proxyModel,
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
        }

        Component.onCompleted: {
            wpItem.filterStrChanged.connect(function() {
                if(this.enabled)
                    filterToList(this.model, this.filterStr)
            }.bind(wpItem));
        }
    }

    FolderListModel {
        id: folderList
        folder: wpItem.enabled ? wpItem.workshopDir : ""
        onStatusChanged: {
            if (wpItem.enabled && this.status === FolderListModel.Ready) {
                const proxyModel = folderWorker.proxyModel;
                const sendMessage = folderWorker.sendMessage.bind(folderWorker);
                new Promise((resolve, reject) => {
                    Common.clearArray(proxyModel);

                    // seems qml's "for" is a function
                    const count = this.count;
                    const get = this.get.bind(this);
                    for(let i=0;i < count;i++) {
                        const v = Object.create(wpItem.itemTemplate);
                        v.workshopid = get(i,"fileName");
                        // use qurl to convert to file://
                        v.path = Qt.resolvedUrl(get(i,"filePath")).toString();
                        proxyModel.push(v);
                    }
                    resolve();
                }).then((value) => {
                    const msg = {
                        action: "loadFolder", 
                        data: proxyModel
                    };
                    sendMessage(msg);
                });
            }
        }
    }
}
