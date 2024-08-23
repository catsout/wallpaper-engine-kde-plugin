#include <QQmlExtensionPlugin>
#include <QQmlEngine>
#include <array>
#include "MpvBackend.hpp"
#include "SceneBackend.hpp"
#include "MouseGrabber.hpp"
#include "TTYSwitchMonitor.hpp"
#include "PluginInfo.hpp"

constexpr std::array<uint, 2> WPVer { 1, 2 };

class Port : public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char* uri) override {
        if (strcmp(uri, "com.github.catsout.wallpaperEngineKde") != 0) return;
        qmlRegisterType<wekde::PluginInfo>(uri, WPVer[0], WPVer[1], "PluginInfo");
        qmlRegisterType<wekde::MouseGrabber>(uri, WPVer[0], WPVer[1], "MouseGrabber");
        qmlRegisterType<scenebackend::SceneObject>(uri, WPVer[0], WPVer[1], "SceneViewer");
        std::setlocale(LC_NUMERIC, "C");
        qmlRegisterType<mpv::MpvObject>(uri, WPVer[0], WPVer[1], "Mpv");
        qmlRegisterType<wekde::TTYSwitchMonitor>(uri, WPVer[0], WPVer[1], "TTYSwitchMonitor");
    }
};

#include "plugin.moc"
