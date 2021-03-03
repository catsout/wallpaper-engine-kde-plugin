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
