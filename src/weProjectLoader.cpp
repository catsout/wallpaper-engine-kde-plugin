#include "weProjectLoader.h"
#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QJsonObject>

WEProject::WEProject(QObject *parent):QObject(parent) {}
WEProject::~WEProject() {}

QList<ViewModel *> WEProject::model() { return m_model; }


QString WEProject::url() { return m_url; }
void WEProject::setUrl(const QString &url) {
	m_url = url;
	updateModel();
	emit urlChanged();
}

void WEProject::updateModel()
{
	m_model.clear();
	
	QDir d(m_url);
	d.setFilter(QDir::Dirs| QDir::NoDotAndDotDot);
	
	const QFileInfoList entrys = d.entryInfoList();
	for(auto& entry:entrys){
		QString path = entry.filePath();
		QFile file(path + "/project.json");
		file.open(QIODevice::ReadOnly);
		QJsonDocument jsDoc = QJsonDocument::fromJson(file.readAll());
		QJsonObject obj = jsDoc.object();
		m_model.append(
				new ViewModel(
					path,
					jsDoc["preview"].toString(),
					jsDoc["type"].toString(),
					jsDoc["title"].toString(),
					jsDoc["file"].toString(),
					jsDoc["workshopid"].toString()
					));
	}
}

