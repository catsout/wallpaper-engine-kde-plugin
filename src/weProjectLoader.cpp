/*
 *  Copyright 2020 catsout  <outl941@163.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

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

