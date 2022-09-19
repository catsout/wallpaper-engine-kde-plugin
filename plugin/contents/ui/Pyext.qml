import QtQuick 2.0
import QtWebSockets 1.0
import org.kde.plasma.core 2.0 as PlasmaCore

import "js/jsonrpc.mjs" as Jsonrpc

Item {
    id: root
    readonly property string file: "plasma/wallpapers/com.github.casout.wallpaperEngineKde/contents/pyext.py"
    readonly property string source: {
        const sh = [
            `EXT=${file}`,
            `WKD="no_pyext_file_found"`,
            "[ -f /usr/share/$EXT ] && WKD=/usr/share/$EXT",
            "[ -f \"$HOME/.local/share/$EXT\" ] && WKD=\"$HOME/.local/share/$EXT\"",
            "[ -f \"$XDG_DATA_HOME/$EXT\" ] && WKD=\"$XDG_DATA_HOME/$EXT\"", 
            `exec python3 "$WKD" "${ws_server.url}"`
        ].join("\n");
        return sh;
    }
    readonly property bool ok: ws_server.socket && ws_server.socket.status == WebSocket.Open

    property string _log
    readonly property string log: _log

    property var commands: []

    readonly property string version: _version

    property string _version: {
        if(ok) {
            ws_server.jrpc.send("version").then(res => { 
                this._version = res.result 
            });
        }
        return '-';
    }
    
    function readfile(path) {
        return ws_server.jrpc.send("readfile", [path]).then((el) => {
            return Qt.atob(el.result);
        });
    }
    function get_dir_size(path, depth=3) {
        return ws_server.jrpc.send("get_dir_size", [path, depth]).then(res => res.result);
    }
    function get_folder_list(path, opt={}) {
        return ws_server.jrpc.send("get_folder_list", [path, opt]).then(res => res.result);
    }
    function read_wallpaper_config(id) {
        return ws_server.jrpc.send("read_wallpaper_config", [id]).then(res => res.result);
    }
    function write_wallpaper_config(id, changed) {
        return ws_server.jrpc.send("write_wallpaper_config", [id, changed]);
    }
    function reset_wallpaper_config(id) {
        return ws_server.jrpc.send("reset_wallpaper_config", [id]);
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
        id: ws_server
        listen: true
        property var socket: { status: WebSocket.Closed }
        property var backmsg: []
        property var jrpc: {
            jrpc = new Jsonrpc.Jsonrpc(sendStr.bind(this), _createTimer);
        }

        onClientConnected: {
            console.error("----python helper connected----")
            this.socket = webSocket;
            webSocket.onTextMessageReceived.connect((message) => {
                ws_server.jrpc.receive(message);
            });
            webSocket.onStatusChanged.connect((status) => {
                if(status != WebSocket.Open) {
                    ws_server.jrpc.rejectUnfinished();
                    console.error("----python helper disconnected----")
                } else {
                    ws_server.dealBackmsg();
                }
            });
            this.dealBackmsg();
        }

        function dealBackmsg() {
            if(!ok) return;
            const m = backmsg;
            backmsg = [];
            m.forEach(el => {
                ws_server.socket.sendTextMessage(el);
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
            console.error(data.stderr);
            console.error(data.stdout);
        }
    }
}
