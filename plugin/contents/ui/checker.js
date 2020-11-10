.pragma library

// Use try Qt.createQmlObject to check if c++ lib is installed
// Use try, as Qt.createQmlObject may break the function
// TODO: add CheckItem for c++ lib object, like mpvChecker{} intead of QtObject{}
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
