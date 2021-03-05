#include "SceneViewer.h"
#include <stdexcept>
#include <QGuiApplication>
#include <QtQml>
#include <QtQuick/QQuickWindow>
#include <QtQuick/QQuickView>
#include <iostream>
#include <string>


int main(int argc, char **argv)
{
	if(argc != 3) {
		std::cerr << "usage: "+ std::string(argv[0]) +" <assets dir> <pkg file>\n";
		return 1;
	}
    QGuiApplication app(argc, argv);

	qmlRegisterType<SceneViewer>("scenetest", 1, 0, "SceneViewer");
    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:///pkg/main.qml"));
    view.show();
	QObject *obj = view.rootObject();
	SceneViewer *sv = obj->findChild<SceneViewer *>();
	sv->setProperty("assets", QUrl::fromLocalFile(argv[1]));
	sv->setProperty("source", QUrl::fromLocalFile(argv[2]));
	sv->setProperty("keepAspect", true);
	sv->setCaptureMouse(true);
    return app.exec();
}
