import QtQuick 2.5

import mpvtest 1.0

Item {
    width: 1280
    height: 720

    MpvObject {
        id: player
        anchors.fill: parent
        logfile: '/home/out/mpv.log'
        mute: false
        volume: 1.0
        Component.onCompleted: {
//            player.command(["script-binding","stats/display-stats-toggle"]);
        }
    }
}
