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
            console.error(e.message);
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

const BYTE_UNITS = [
	'B',
	'kB',
	'MB',
	'GB',
	'TB',
	'PB',
	'EB',
	'ZB',
	'YB',
];

export function prettyBytes(number, maxFrac=0) {
    const UNITS = BYTE_UNITS;
	const exponent = number < 1 
        ? 0 
        : Math.min(Math.floor(Math.log(number) / Math.log(1024)), UNITS.length - 1);
    const unit = UNITS[exponent];
    const prefix = number < 0 ? '-' : '';

	const num_str = (number / 1024 ** exponent).toFixed(maxFrac);
    return `${prefix}${num_str} ${unit}`; 
}

