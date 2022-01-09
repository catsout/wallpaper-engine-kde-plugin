import QtQuick 2.5
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.2
import QtQuick.Window 2.1
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.taskmanager 0.1 as TaskManager

/*
https://github.com/KDE/plasma-workspace/blob/master/libtaskmanager/abstracttasksmodel.h
    enum AdditionalRoles {
        AppId = Qt::UserRole + 1, //< KService storage id (.desktop name sans extension). 
        AppName, //< Application name. 
        GenericName, //< Generic application name. 
        LauncherUrl, //< URL that can be used to launch this application (.desktop or executable). 
        LauncherUrlWithoutIcon, //< Special path to get a launcher URL while skipping fallback icon encoding. Used as speed optimization. 
        WinIdList, //< NOTE: On Wayland, these ids are only useful within the same process. On X11, they are global window ids. 
        MimeType, //< MIME type for this task (window, window group), needed for DND. 
        MimeData, //< Data for MimeType. 
        IsWindow, //< This is a window task. 
        IsStartup, //< This is a startup task. 
        IsLauncher, //< This is a launcher task. 
        HasLauncher, //< A launcher exists for this task. Only implemented by TasksModel, not by either the single-type or munging tasks models. 
        IsGroupParent, //< This is a parent item for a group of child tasks. 
        ChildCount, //< The number of tasks in this group. 
        IsGroupable, //< Whether this task is being ignored by grouping or not. 
        IsActive, //< This is the currently active task. 
        IsClosable, //< requestClose (see below) available. 
        IsMovable, //< requestMove (see below) available. 
        IsResizable, //< requestResize (see below) available. 
        IsMaximizable, //< requestToggleMaximize (see below) available. 
        IsMaximized, //< Task (i.e. window) is maximized. 
        IsMinimizable, //< requestToggleMinimize (see below) available. 
        IsMinimized, //< Task (i.e. window) is minimized. 
        IsKeepAbove, //< Task (i.e. window) is keep-above. 
        IsKeepBelow, //< Task (i.e. window) is keep-below. 
        IsFullScreenable, //< requestToggleFullScreen (see below) available. 
        IsFullScreen, //< Task (i.e. window) is fullscreen. 
        IsShadeable, //< requestToggleShade (see below) available. 
        IsShaded, //< Task (i.e. window) is shaded. 
        IsVirtualDesktopsChangeable, //< requestVirtualDesktop (see below) available. 
        VirtualDesktops, //< Virtual desktops for the task (i.e. window). 
        IsOnAllVirtualDesktops, //< Task is on all virtual desktops. 
        Geometry, //< The task's geometry (i.e. the window's). 
        ScreenGeometry, //< Screen geometry for the task (i.e. the window's screen). 
        Activities, //< Activities for the task (i.e. window). 
        IsDemandingAttention, //< Task is demanding attention. 
        SkipTaskbar, //< Task should not be shown in a 'task bar' user interface. 
        SkipPager, //< Task should not to be shown in a 'pager' user interface. 
        AppPid, //< Application Process ID
        StackingOrder, //< A window task's index in the window stacking order. 
        LastActivated, //< The timestamp of the last time a task was the active task. 
        ApplicationMenuServiceName, //< The DBus service name for the application's menu. May be empty. @since 5.19 
        ApplicationMenuObjectPath, //< The DBus object path for the application's menu. May be empty. @since 5.19 
        IsHidden, //< Task (i.e window) is hidden on screen. A minimzed window is not necessarily hidden. 
    };
*/

Item {

    id: wModel
    property bool playVideoWallpaper: true
    property bool logging: false
    property var desktop: 0

    property string activity
    property var screenGeometry

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

    TaskManager.ActivityInfo { 
        id: activityInfo 
        onCurrentActivityChanged: virtualDesktopInfo.onCurrentDesktopChanged();
    }

    TaskManager.VirtualDesktopInfo { 
        id: virtualDesktopInfo 
        onCurrentDesktopChanged: {
            if(activity === activityInfo.currentActivity)
                wModel.desktop = this.currentDesktop;
        }
        Component.onCompleted: {
            wModel.desktop = this.currentDesktop;
        }
    }
    TaskManager.TasksModel {
        id: tasksModel
        sortMode: TaskManager.TasksModel.SortVirtualDesktop
        groupMode: TaskManager.TasksModel.GroupDisabled

        virtualDesktop: wModel.desktop
        filterByVirtualDesktop: true

        screenGeometry: wModel.screenGeometry
        filterByScreen: true

        // demandingAttentionSkipsFilters not available here, which may cause, 
        // skip activity filter, so filter activties manually
        //demandingAttentionSkipsFilters: false
        //activity: wModel.activity
        //filterByActivity: true

        onActiveTaskChanged: updateWindowsinfo();
        onVirtualDesktopChanged: {
            if(wModel.logging)
                console.error(this.virtualDesktop, ':', this.screenGeometry)
            updateWindowsinfo();
        }
        function getProperty(idx, property) {
            if(TaskManager.AbstractTasksModel[property] === undefined) return undefined;
            return this.data(idx, TaskManager.AbstractTasksModel[property]);
        }
    }
    PlasmaCore.SortFilterModel {
        filterRole: 'IsWindow'
        filterRegExp: 'true'
        sourceModel: tasksModel
        onDataChanged: updateWindowsinfo()
        onCountChanged: updateWindowsinfo()
    }

    Timer{
        id: triggerTimer
        running: false
        repeat: false
        interval: 200
        onTriggered: {
            _updateWindowsinfo();
        }
    }

    /*
    filters {
        IsWindows: true
    }
    callback(getproperty) { return getproperty("IsWindows") === true }
    */
    function filterTaskModel(model, filters, callback) {
        function doCallback(idx) {
            return callback((property) => model.getProperty(idx, property));
        }
        function filter(idx) {
            for(const [key, value] of Object.entries(filters)) {
                const property = model.getProperty(idx, key);
                if(property !== undefined && property !== value)
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
                    const idx = tasksModel.makeModelIndex(i);
                    if(filter(idx)) 
                        result.push(i);
                }
                return result;
            },
            filterExist: function(array) {
                const result = [];
                array.forEach((el) => {
                    const idx = tasksModel.makeModelIndex(el);
                    if(filter(idx)) 
                        result.push(el);
                });
                return result;
            }
        };
    }

    function updateWindowsinfo() {
        triggerTimer.start();
    }
    function _updateWindowsinfo() {
        if(modePlay === Common.PauseMode.Never) {
            play();
            return;
        }
        const basefilters = {
            IsWindow: true,
            SkipTaskbar: false,
            SkipPager: false
        };
        const baseWModel = filterTaskModel(tasksModel, {}, (getproperty) => {
            const activities = getproperty("Activities");
            if(activities && activities.length) {
                for(let i=0;i < activities.length;i++) {
                    if(activities[i] === wModel.activity)
                        return true;
                }
                return false;
            }
            // true when model don't have activities info
            return true;
        }).filterExist(filterTaskModel(tasksModel, basefilters).filter());
        const notMinWModel = filterTaskModel(tasksModel, {IsMinimized: false}).filterExist(baseWModel);

        // can simply skip when "activity != currentActivity", but use full filter
        //const notMinWModel = filterTaskModel(tasksModel, Object.assign({IsMinimized: false}, basefilters)).filter();
        const maxWModel = filterTaskModel(tasksModel, {}, (getproperty) => {
            return getproperty("IsMaximized") === true || getproperty("IsFullScreen") === true;
        }).filterExist(notMinWModel);

        if(modePlay === Common.PauseMode.Any) {
            playBy(notMinWModel.length == 0 ? true : false);
        }
        else {
            playBy(maxWModel.length == 0 ? true : false);
        }

        if(logging) {
            const printW = (i) => {
                const idx = tasksModel.makeModelIndex(i);
                console.error(tasksModel.data(idx, TaskManager.AbstractTasksModel.AppName));
                console.error(tasksModel.data(idx, TaskManager.AbstractTasksModel.Activities));
            }
            console.error("--------Not Minimized--------");
            notMinWModel.forEach(printW);
            console.error("---------Maximized----------");
            maxWModel.forEach(printW);
        }
    }
}

