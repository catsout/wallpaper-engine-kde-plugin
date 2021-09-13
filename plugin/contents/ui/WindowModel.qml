import QtQuick 2.5
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.2
import QtQuick.Window 2.1
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.taskmanager 0.1 as TaskManager

Item {

    id: wModel
    property bool playVideoWallpaper: true
    property bool logging: false

    property int modePlay: wallpaper.configuration.PauseMode

    Timer{
        id: playTimer
        running: false
        repeat: false
        interval: 400
        onTriggered: {
            playVideoWallpaper = true;
        }
    }
    function play() {
        playTimer.stop();
        playTimer.start();
    }
    function pause() {
        playTimer.stop();
        playVideoWallpaper = false;
    }
    function playBy(value) {
        if(value) play();
        else pause();
    }

    TaskManager.VirtualDesktopInfo { id: virtualDesktopInfo }
    TaskManager.ActivityInfo { id: activityInfo }
    TaskManager.TasksModel {
        id: tasksModel
        sortMode: TaskManager.TasksModel.SortVirtualDesktop
        groupMode: TaskManager.TasksModel.GroupDisabled

        activity: activityInfo.currentActivity
        virtualDesktop: virtualDesktopInfo.currentDesktop
        screenGeometry: wallpaper.parent.screenGeometry ? wallpaper.parent.screenGeometry : Qt.rect(0,0,0,0) // Warns "Unable to assign [undefined] to QRect" during init, but works thereafter.

        filterByActivity: true
        filterByVirtualDesktop: true
        filterByScreen: true

        //onActiveTaskChanged: updateWindowsinfo(wModel.modePlay)
        onDataChanged: updateWindowsinfo(wModel.modePlay)
        onVirtualDesktopChanged: updateWindowsinfo(wModel.modePlay)
    }
    /*
    filters {
        IsWindows: true
    }
    callback(getproperty) { return getproperty("IsWindows") === true }
    */
    function filterTaskModel(model, filters, callback) {
        function doCallback(idx) {
            return callback((property) => model.data(idx, TaskManager.AbstractTasksModel[property]));
        }
        function filter(idx) {
            for(const [key, value] of Object.entries(filters)) {
                if(model.data(idx, TaskManager.AbstractTasksModel[key]) != value)
                    return false;
            };
            if(callback) {
                if(!doCallback(idx)) return false;
            }
            return true;
        }
        return {
            filter: function() {
                const result = [];
                for (let i = 0 ; i < model.count; i++) {
                    let idx = tasksModel.makeModelIndex(i);
                    if(filter(idx)) 
                        result.push(i);
                }
                return result;
            },
            filterExist: function(array) {
                const result = [];
                array.forEach((el) => {
                    let idx = tasksModel.makeModelIndex(el);
                    if(filter(idx)) 
                        result.push(el);
                });
                return result;
            }
        };
    }

    function updateWindowsinfo(modePlay) {
        if(modePlay === Common.PauseMode.Never) {
            play();
            return;
        }
        const basefilters = {
            IsWindow: true,
            SkipTaskbar: false,
            SkipPager: false
        };
        const notMinWModel = filterTaskModel(tasksModel, Object.assign({IsMinimized: false}, basefilters)).filter();
        const maxWModel = filterTaskModel(tasksModel, {}, (getproperty) => {
            return getproperty("IsMaximized") === true || getproperty("IsFullScreen") === true;
        }).filterExist(notMinWModel);

        if(logging) {
            const printW = (i) => {
                const idx = tasksModel.makeModelIndex(i);
                console.log(tasksModel.data(idx, TaskManager.AbstractTasksModel.AppName));
            }
            console.log("--------Not Minimized--------");
            notMinWModel.forEach(printW);
            console.log("---------Maximized----------");
            maxWModel.forEach(printW);
        }
        if(modePlay === Common.PauseMode.Any) {
            playBy(notMinWModel.length == 0 ? true : false);
        }
        else {
            playBy(maxWModel.length == 0 ? true : false);
        }
    }
}

