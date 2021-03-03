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
