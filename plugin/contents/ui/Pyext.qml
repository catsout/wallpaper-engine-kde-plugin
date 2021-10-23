import QtQuick 2.0
import QtWebSockets 1.0
import org.kde.plasma.core 2.0 as PlasmaCore

import "jsonrpc.mjs" as Jsonrpc

Item {
    id: root
    readonly property string file: "plasma/wallpapers/com.github.casout.wallpaperEngineKde/contents/pyext.py"
    readonly property string source: {
        const sh = [
            `WKD=${file}`,
            "[ \"$XDG_DATA_HOME\" ] && WKD=$XDG_DATA_HOME/$WKD || WKD=$HOME/.local/share/$WKD",
            `exec python3 $WKD ${server.url}`
        ].join("\n");
        return sh;
    }
    readonly property bool ok: server.socket.status == WebSocket.Open

    property string _log
    readonly property string log: _log

    property var commands: []

    function readfile(path) {
        return server.jrpc.send("readfile", [path]).then((el) => {
            return Qt.atob(el.result);
        });
    }

    function _createTimer(callback) {
        const timer = Qt.createQmlObject("import QtQuick 2.0; Timer {}", root);
        const interval = 500;
        timer.interval = interval;
        timer.repeat = true;
        timer.triggered.connect(() => callback(500));
        timer.start();
        return timer;
    }

    WebSocketServer {
        id: server
        listen: true
        property var socket: { status: WebSocket.Closed }
        property var backmsg: []
        property var jrpc: new Jsonrpc.Jsonrpc(sendStr.bind(this), _createTimer);

        onClientConnected: {
            this.socket = webSocket;
            webSocket.onTextMessageReceived.connect((message) => {
                this.jrpc.reserve(message);
            });
            webSocket.onStatusChanged.connect((status) => {
                if(status != WebSocket.Open) {
                    this.jrpc.rejectUnfinished();
                } else {
                    this.dealBackmsg();
                }
            });
            this.dealBackmsg();
        }

        function dealBackmsg() {
            if(!ok) return;
            const m = backmsg;
            backmsg = [];
            m.forEach(el => {
                server.socket.sendTextMessage(el);
            });
        }
        function sendStr(s) {
            if(!ok) {
                this.backmsg.push(s);
            } else {
                this.socket.sendTextMessage(s);
            }
        }
    }

    PlasmaCore.DataSource {
        engine: 'executable'
        connectedSources: [source]
        onNewData: {
            _log += "\n" + data.stderr;
            _log += "\n" + data.stdout;
        }
    }
}
