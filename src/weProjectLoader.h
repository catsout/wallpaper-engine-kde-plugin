
#ifndef WE_PROJECTLOADER
#define WE_PROJECTLOADER

#include <QLibrary>
#include <QJsonDocument>

#include "viewModel.h"

class WEProject : public QObject
{
	Q_OBJECT
		Q_PROPERTY(QList<ViewModel *> model READ model NOTIFY modelChanged)
		Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)

	public:
		WEProject(QObject *parent=0);
		~WEProject() override;
		QList<ViewModel *> model();
		QString url();
		void setUrl(const QString &url);

signals:
		void modelChanged();
		void urlChanged();
	private:
		QString m_url = "";
		QList<ViewModel *> m_model = {};
		void updateModel();
};

#endif
