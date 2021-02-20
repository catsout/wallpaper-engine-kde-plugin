/*
 *  Copyright 2020 catsout  <outl941@163.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.7
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.3
import QtQuick.Window 2.1
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.taskmanager 0.1 as TaskManager

Item {

    id: wModel
    property alias screenGeometry: tasksModel.screenGeometry
    property bool playVideoWallpaper: true
    property bool currentWindowMaximized: false
    property bool isActiveWindowPinned: false
    property int modePlay: wallpaper.configuration.PauseMode

    TaskManager.VirtualDesktopInfo { id: virtualDesktopInfo }
    TaskManager.ActivityInfo { id: activityInfo }
    TaskManager.TasksModel {
        id: tasksModel
        sortMode: TaskManager.TasksModel.SortVirtualDesktop
        groupMode: TaskManager.TasksModel.GroupDisabled

        activity: activityInfo.currentActivity
        virtualDesktop: virtualDesktopInfo.currentDesktop
        screenGeometry: wallpaper.screenGeometry // Warns "Unable to assign [undefined] to QRect" during init, but works thereafter.

        filterByActivity: true
        filterByVirtualDesktop: true
        filterByScreen: true

        onActiveTaskChanged: updateWindowsinfo(wModel.modePlay)
        onDataChanged: updateWindowsinfo(wModel.modePlay)
        Component.onCompleted: {
            maximizedWindowModel.sourceModel = tasksModel
            fullScreenWindowModel.sourceModel = tasksModel
            minimizedWindowModel.sourceModel = tasksModel
            onlyWindowsModel.sourceModel = tasksModel
        }
    }

    PlasmaCore.SortFilterModel {
        id: onlyWindowsModel
        filterRole: 'IsWindow'
        filterRegExp: 'true'
        onDataChanged: updateWindowsinfo(wModel.modePlay)
        onCountChanged: updateWindowsinfo(wModel.modePlay)
    }

    PlasmaCore.SortFilterModel {
        id: maximizedWindowModel
        filterRole: 'IsMaximized'
        filterRegExp: 'true'
        onDataChanged: updateWindowsinfo(wModel.modePlay)
        onCountChanged: updateWindowsinfo(wModel.modePlay)
    }
    PlasmaCore.SortFilterModel {
        id: fullScreenWindowModel
        filterRole: 'IsFullScreen'
        filterRegExp: 'true'
        onDataChanged: updateWindowsinfo(wModel.modePlay)
        onCountChanged: updateWindowsinfo(wModel.modePlay)
    }

    PlasmaCore.SortFilterModel {
        id: minimizedWindowModel
        filterRole: 'IsMinimized'
        filterRegExp: 'true'
        onDataChanged: updateWindowsinfo(wModel.modePlay)
        onCountChanged: updateWindowsinfo(wModel.modePlay)
    }

    function updateWindowsinfo(modePlay) {
        //0 for maxizied
        //1 for any
        //3 for never
        if(modePlay === 1) {
            playVideoWallpaper = (onlyWindowsModel.count === minimizedWindowModel.count) ? true : false;
        }
        else if(modePlay === 2) {
            playVideoWallpaper = true;
        }
        else {
            var joinApps  = [];
            var minApps  = [];
            var aObj;
            var i;
            var j;
            // add fullscreen apps
            for (i = 0 ; i < fullScreenWindowModel.count ; i++){
                aObj = fullScreenWindowModel.get(i)
                joinApps.push(aObj.AppPid)
            }
            // add maximized apps
            for (i = 0 ; i < maximizedWindowModel.count ; i++){
                aObj = maximizedWindowModel.get(i)
                joinApps.push(aObj.AppPid)                
            }

            // add minimized apps
            for (i = 0 ; i < minimizedWindowModel.count ; i++){
                aObj = minimizedWindowModel.get(i)
                minApps.push(aObj.AppPid)
            }
            
            joinApps = removeDuplicates(joinApps) // for qml Kubuntu 18.04
            
            joinApps.sort();
            minApps.sort();

            var twoStates = 0
            j = 0;
            for(i = 0 ; i < minApps.length ; i++){
                if(minApps[i] === joinApps[j]){
                    twoStates = twoStates + 1;
                    j = j + 1;
                }
            }
            playVideoWallpaper = (fullScreenWindowModel.count + maximizedWindowModel.count - twoStates) == 0 ? true : false
        }
    }
    
    function removeDuplicates(arrArg){
        return arrArg.filter(function(elem, pos,arr) {
                        return arr.indexOf(elem) == pos;
                });
    }
}

