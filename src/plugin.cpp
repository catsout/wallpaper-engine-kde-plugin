#include <QQmlExtensionPlugin>
#include <QQmlEngine>
#include "weProjectLoader.h"
#include "mpvbackend.h"
#include "SceneViewer.h"

class Port : public QQmlExtensionPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri)
    {  
		if(strcmp(uri, "com.github.catsout.wallpaperEngineKde") != 0)
			return;
		qmlRegisterType<WEProject>(uri, 1, 0, "WEProject");
		qmlRegisterType<SceneViewer>(uri, 1, 0, "SceneViewer");
		std::setlocale(LC_NUMERIC, "C");
		qmlRegisterType<MpvObject>(uri, 1, 0, "MpvObject");
    }   
};

#include "plugin.moc"
