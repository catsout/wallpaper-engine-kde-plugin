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

#include "viewModel.h"
ViewModel::ViewModel(QObject *parent):QObject(parent) {}
ViewModel::ViewModel(
		const QString &path, 
		const QString &preview, 
		const QString &type,
		const QString &title,
		const QString &file,
		const QString &workshopid,
		QObject *parent)
	:
		QObject(parent), 
		m_path(path), 
		m_preview(preview), 
		m_type(type), 
		m_title(title), 
		m_file(file),
		m_workshopid(workshopid) {}

		ViewModel::~ViewModel(){} 

		QString ViewModel::path() const { return m_path; }


QString ViewModel::preview() const { return m_preview; }
QString ViewModel::type() const { return m_type; }
QString ViewModel::title() const { return m_title; }
QString ViewModel::file() const { return m_file; }
QString ViewModel::workshopid() const { return m_workshopid; }
