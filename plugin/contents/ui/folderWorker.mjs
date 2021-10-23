import { parseJson } from "utils.mjs";

function readCallback(text, el) {
    const project = parseJson(text);    
    if(project !== null) {
        if("title" in project)
            el.title = project.title;
        if("preview" in project && project.preview)
            el.preview = project.preview;
        if("file" in project)
            el.file = project.file;
        if("type" in project)
            el.type = project.type.toLowerCase();
        if("contentrating" in project)
            el.contentrating = project.contentrating;
        if("tags" in project)
            el.tags = project.tags;
    }
}

function genFilter(filters) {
    const typeF = {};
    const noTags = new Set();
    let onlyFavor = false;
    filters.forEach((el) => {
        if(el.type === "type") 
            typeF[el.key] = el.value;
        else if(el.type === "contentrating")
            typeF[el.key] = el.value;
        else if(el.type === "favor") 
            onlyFavor = el.value;
        else if(el.type === "tags") {
            if(!el.value) noTags.add(el.key)
        }
    });

    const checkType = (el) => Boolean(typeF[el.type]);
    const checkContentrating = (el) => Boolean(typeF[el.contentrating]);
    const checkFavor = (el) => onlyFavor?el.favor:true;
    const checkNoTags = (el) => {
        for(let i=0;i < el.tags.length;i++) {
            if(noTags.has(el.tags[i])) {
                return false;
            }
        }
        return true;
    }
    return (el) => {
        return checkType(el) && checkFavor(el) && checkContentrating(el) && checkNoTags(el);
    }
}

WorkerScript.onMessage = function(msg) {
    const reply = WorkerScript.sendMessage;
    if(msg.action == "loadFolder") {
        /*
        const data = msg.data;
        const plist = [];
        msg.data.forEach((el) => {
            const p = readTextFile(el.path + "/project.json").then(value => {
                    readCallback(value.response, el);
                }).catch(reason => console.log(reason));
            plist.push(p);
        });
        Promise.all(plist).then(value => {
            reply({ "reply": msg.action, "data": data, "path": msg.path });
        });
        */
    }
    else if(msg.action == "filter") {
        const filter = genFilter(msg.filters);
        const model = msg.model;
        model.clear();
        msg.data.forEach(function(el) {
            if(filter(el))
                model.append(el);
        });
        model.sync();
        reply({ "reply": msg.action });
    }
}
