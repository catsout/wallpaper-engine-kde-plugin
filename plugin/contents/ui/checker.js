.pragma library

function checklib(libName, parentItem) {
	var ok = false;
	var create = null;
	 try {
		create = Qt.createQmlObject(
		'import '+ libName +';import QtQml 2.13;QtObject{}',
		parentItem);

	} catch (error) {}
	if(create != null){
		ok = true;
		create.destroy(1000);
	}
	return ok;
}

function checklib_wallpaper(parentItem) {
	return checklib('com.github.catsout.wallpaperEngineKde 1.0', parentItem);
}

function checklib_folderlist(parentItem) {
	return checklib('Qt.labs.folderlistmodel 2.12', parentItem)
}
