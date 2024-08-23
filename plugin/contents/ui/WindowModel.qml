import QtQuick 2.5
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2
import QtQuick.Window 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasma5support as Plasma5Support

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
    property bool logging: false
    property var desktop: 0

    property string activity
    property var screenGeometry

    property alias filterByScreen: tasksModel.filterByScreen
    property alias resumeTime: playTimer.interval
    property int modePlay

    // ---
    readonly property bool reqPause: _reqPause
    property bool _reqPause: false

    Timer{
        id: playTimer
        running: false
        repeat: false
        onTriggered: {
            _reqPause = false;
        }
    }
    function play() {
        playTimer.stop();
        playTimer.start();
    }
    function pause() {
        playTimer.stop();
        _reqPause = true;
    }
    function playBy(value) {
        if(value) play();
        else pause();
    }

    TaskManager.ActivityInfo { 
        id: activityInfo 
        onCurrentActivityChanged: virtualDesktopInfo.onCurrentDesktopChanged();
        Component.onCompleted: {
            wModel.activity = this.currentActivity;
        }
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

        // in alias
        // filterByScreen: true

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

    Plasma5Support.SortFilterModel {
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
        interval: 100
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
    function genTaskModelFilter(model) {
        function doCallback(idx, callback) {
            return callback((property) => model.getProperty(idx, property));
        }
        function filter(callback, inArray) {
            const array = inArray ? inArray : Array.from(Array(model.count).keys());
            return array.filter(el => {
                const idx = model.makeModelIndex(el);
                return doCallback(idx, callback);
            });
        }
        return {
            filter: function(filters, array) {
                return filter((getProperty) => {
                    const nfilters = filters ? filters : {};
                    for(const [key, value] of Object.entries(nfilters)) {
                        const property = getProperty(key);
                        if(property !== undefined && property !== value)
                            return false;
                    };
                    return true;
                }, array);
            },
            filterCallback: function(callback, array) {
                return filter(callback, array);
            }
        };
    }

    function updateWindowsinfo() {
        triggerTimer.start();
    }
    function _updateWindowsinfo() {
        if(modePlay === Common.PauseMode.Never) {
            playBy(true);
            return;
        }
        const basefilters = {
            IsWindow: true,
//            SkipTaskbar: false,
//            SkipPager: false
        };
        const taskFilter = genTaskModelFilter(tasksModel);
        const baseWModel = taskFilter.filterCallback((getproperty) => {
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
        }, taskFilter.filter(basefilters));
        const notMinWModel = taskFilter.filter({IsMinimized: false}, baseWModel);

        // can simply skip when "activity != currentActivity", but use full filter
        //const notMinWModel = filterTaskModel(tasksModel, Object.assign({IsMinimized: false}, basefilters)).filter();
        const maxWModel = taskFilter.filterCallback((getproperty) => {
            return getproperty("IsMaximized") === true || getproperty("IsFullScreen") === true;
        }, notMinWModel);

        const fullSModel = taskFilter.filterCallback((getproperty) => {
            return getproperty("IsFullScreen") === true;
        }, notMinWModel);

        const activeModel = taskFilter.filter({IsActive: true}, notMinWModel);


        switch (modePlay) {
        case Common.PauseMode.FocusOrMax:
            playBy(maxWModel.length === 0 && activeModel.length === 0);
            break;
        case Common.PauseMode.Any:
            playBy(notMinWModel.length === 0);
            break;
        case Common.PauseMode.Max:
            playBy(maxWModel.length === 0);
            break;
        case Common.PauseMode.FullScreen:
            console.log("fullScreen", fullSModel);
            playBy(fullSModel.length === 0);
            break;
        case Common.PauseMode.Focus:
            playBy(activeModel.length === 0);
            break;
        default:
            playBy(true);
        }

        if(logging) {
            const printW = (i) => {
                const idx = tasksModel.makeModelIndex(i);
                const name = tasksModel.data(idx, TaskManager.AbstractTasksModel.AppName);
                const activity = tasksModel.data(idx, TaskManager.AbstractTasksModel.Activities);
                console.error("--", name, activity);
            }
            console.error("--------Not Minimized--------");
            notMinWModel.forEach(printW);
            console.error("---------Maximized----------");
            maxWModel.forEach(printW);
            console.error("-----------Active-----------");
            activeModel.forEach(printW);
            console.error("\n\n\n")
        }
    }
}

