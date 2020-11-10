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

#ifndef WE_VIEWMODEL
#define WE_VIEWMODEL
#include <QLibrary>

class ViewModel : public QObject
{
	Q_OBJECT
		Q_PROPERTY(QString path READ path)
		Q_PROPERTY(QString preview READ preview)
		Q_PROPERTY(QString type READ type)
		Q_PROPERTY(QString title READ title)
		Q_PROPERTY(QString file READ file)
		Q_PROPERTY(QString workshopid READ workshopid)


	public:
		ViewModel(QObject *parent=0);

		ViewModel(const QString &path, 
			const QString &preview, 
			const QString &type,
			const QString &title,
			const QString &file,
			const QString &workshopid,
			QObject *parent=0);

		~ViewModel() override;
		QString path() const;
		QString preview() const;
		QString type() const;
		QString title() const;
		QString file() const;
		QString workshopid() const;

	signals:

	private:
		QString m_path;
		QString m_preview;
		QString m_type;
		QString m_title;
		QString m_file;
		QString m_workshopid;
};
#endif
