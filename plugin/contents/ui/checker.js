.pragma library

function checklib(parentItem) {
	var ok = false;
	var create = null;
	try {
		create = Qt.createQmlObject(
		'import wallpaper.engineforkde 1.0;import QtQml 2.13;QtObject{}',
		parentItem);

	} catch (error) {}
	if(create != null){
		ok = true;
		create.destroy(1000);
	}
	return ok;
}
