import { readTextFile,parseJson } from "utils.mjs";

function readCallback(text, el) {
    let project = parseJson(text);    
    if(project !== null) {
        if("title" in project)
            el.title = project.title;
        if("preview" in project)
            el.preview = project.preview;
        if("file" in project)
            el.file = project.file;
        if("type" in project)
            el.type = project.type.toLowerCase();
    }
}

WorkerScript.onMessage = function(msg) {
    let reply = WorkerScript.sendMessage;
    if(msg.action == "loadFolder") {
        const data = msg.data;
        const plist = [];
        data.forEach(function(el) {
			// as no allSettled, catch any error
            let p = readTextFile(el.path + "/project.json").then(value => {
                    readCallback(value.response, el);
                }).catch(reason => console.log(reason));
            plist.push(p);
        });
        Promise.all(plist).then(value => {
            reply({ "reply": msg.action, "data": data, "path": msg.path });
        });
    }
    else if(msg.action == "filter") {
        const typeFilter = (() => {
            const obj = {};
            msg.filters.map((el) => {
                if(el.type == "type") obj[el.key] = el.value;
            });
            return (str) => obj[str];
        })();
        const model = msg.model;
        model.clear();
        msg.data.forEach(function(el) {
            if(typeFilter(el.type))
                model.append(el);
        });
        model.sync();
        reply({ "reply": msg.action });
    }
}
