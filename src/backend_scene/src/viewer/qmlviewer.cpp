#include "SceneBackend.h"
#include <QGuiApplication>
#include <QtQml>
#include <QtQuick/QQuickView>
#include <iostream>
#include <string>


int main(int argc, char **argv)
{
	if(argc != 3) {
		std::cerr << "usage: "+ std::string(argv[0]) +" <assets dir> <pkg file>\n";
		return 1;
	}
	QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    QGuiApplication app(argc, argv);

	qmlRegisterType<SceneObject>("scenetest", 1, 0, "SceneViewer");
    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:///pkg/main.qml"));
    view.show();
	QObject *obj = view.rootObject();
	SceneObject *sv = obj->findChild<SceneObject *>();
	sv->setProperty("assets", QUrl::fromLocalFile(argv[1]));
	sv->setProperty("source", QUrl::fromLocalFile(argv[2]));
	//sv->setAcceptMouse(true);
	//sv->setAcceptHover(true);
    return QGuiApplication::exec();
}
