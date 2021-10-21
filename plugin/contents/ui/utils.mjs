/*
export function readTextFile(fileUrl) {
    return new Promise(function (resolve, reject) {
        const request = new XMLHttpRequest;
		// Setup listener
		request.onreadystatechange = function () {
			if (request.readyState !== XMLHttpRequest.DONE) return;

			// Process the response
			if (request.status >= 200 && request.status < 300) {
				// If successful
				resolve(request);
			} else {
				// If failed
				reject(`failed load file(${request.status}): ${fileUrl}`);
			}

		};
        request.open("GET", fileUrl);
		request.send();
	});
}
*/

export function parseJson(str) {
    let obj_j;
    try {
        obj_j = JSON.parse(str);
    } catch (e) {
        if (e instanceof SyntaxError) {
            console.log(e.message);
            obj_j = null;
        } else {
          throw e;  // re-throw the error unchanged
        }
    } 
    return obj_j;
}

export function basename(path) {
    return path.split('/').reverse()[0];
}

export function dirname(path) {
    return path.substring(0, str.lastIndexOf("/"));
}

export function trimCharR(str, c) {
    let pos = 0;
    while(str.slice(pos - 1, str.length + pos) === c) {
        pos -= 1;
    }
    return str.slice(0, str.length + pos);
}


export function strToIntArray(str) {
    return [...str].map((e) => e.charCodeAt(0) - '0'.charCodeAt(0));
}
export function intArrayToStr(arr) {
    return arr.reduce((acc, e) => acc + e.toString(), "");
}


