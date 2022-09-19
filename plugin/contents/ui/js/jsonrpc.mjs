import { parseJson } from "utils.mjs";


export function Jsonrpc(sendstr, createTimer) {
    this.id = 0;
    this._sendstr = sendstr;
    this._pMap = new Map();
    this._timer = createTimer((pass) => {
        const keys = []
        this._pMap.forEach(function(value, key) {
            value.time -= pass;
            if(value.time <= 0) { 
                keys.push(key);
                value.reject("time out");
            }
        });
        keys.forEach((el) => this._pMap.delete(el));
    });
}

Jsonrpc.prototype.rejectUnfinished = function() {
    this._pMap.forEach(function(value, key) {
        value.reject("");
    });
    this._pMap.clear();
}

Jsonrpc.prototype.send = function(method, params) {
    const id = this.id++;
    return new Promise((resolve, reject) => {
        this._pMap.set(id, {
            resolve: resolve,
            reject: reject,
            time: 5000
        });
        this._sendstr(JSON.stringify({
            method: method,
            params: params,
            id: id
        }));
    });
}

Jsonrpc.prototype.receive = function(str) {
    const obj = parseJson(str);
    if(!obj) {
        return;
    }
    const id = obj.id;
    const error = obj.error;
    if(this._pMap.has(id)) {
        const p = this._pMap.get(id);
        if(error) p.reject(error)
        else p.resolve(obj);
        this._pMap.delete(id);
    } else if (id === -1) {
        console.error(error);
    } else {
        console.error("received timeout result, id:", id);
    }
}
