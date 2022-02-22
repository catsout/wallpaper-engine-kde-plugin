#include <stdexcept>
#include <QGuiApplication>
#include <QtQml>
#include <QtQuick/QQuickWindow>
#include <QtQuick/QQuickView>
#include <iostream>
#include <string>
#include "mpvbackend.h"

int main(int argc, char **argv)
{
	if(argc != 2) {
		std::cerr << "usage: "+ std::string(argv[0]) +" <video file>\n";
		return 1;
	}
	QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    QGuiApplication app(argc, argv);
	qmlRegisterType<MpvObject>("mpvtest", 1, 0, "MpvObject");
	setlocale(LC_NUMERIC, "C");
    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:///qml/main.qml"));
    view.show();
	QObject *obj = view.rootObject();
	MpvObject* mpv = obj->findChild<MpvObject *>();
	mpv->setSource(QUrl::fromLocalFile(argv[1]));
    return app.exec();
}
