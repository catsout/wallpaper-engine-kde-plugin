#include <QQmlExtensionPlugin>
#include <QQmlEngine>
#include <array>
#include "mpvbackend.h"
#include "SceneBackend.h"
#include "MouseGrabber.h"
#include "WPCommon.h"

constexpr std::array<uint,2> WPVersion {1, 1};

class Port : public QQmlExtensionPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override
    {  
		if(strcmp(uri, "com.github.catsout.wallpaperEngineKde") != 0)
			return;
		qmlRegisterType<MouseGrabber>(uri, WPVersion[0], WPVersion[1], "MouseGrabber");
		qmlRegisterType<scenebackend::SceneObject>(uri, WPVersion[0], WPVersion[1], "SceneViewer");
		std::setlocale(LC_NUMERIC, "C");
		qmlRegisterType<MpvObject>(uri, WPVersion[0], WPVersion[1], "MpvObject");
    }   
};

#include "plugin.moc"
