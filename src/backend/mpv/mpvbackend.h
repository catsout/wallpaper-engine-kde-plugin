#ifndef MPVRENDERER_H_
#define MPVRENDERER_H_

#include <QtQuick/QQuickFramebufferObject>

#include <mpv/client.h>
#include <mpv/render_gl.h>
#include "qthelper.hpp"

class MpvRenderer;

class MpvObject : public QQuickFramebufferObject
{
    Q_OBJECT
	Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
	Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool mute READ mute WRITE setMute)
	Q_PROPERTY(QString logfile READ logfile WRITE setLogfile)
	Q_PROPERTY(int volume READ volume WRITE setVolume)


    mpv_handle *mpv;

    friend class MpvRenderer;

public:
    static void on_update(void *ctx);

    MpvObject(QQuickItem * parent = 0);
    virtual ~MpvObject();
    virtual Renderer *createRenderer() const;
	
	enum Status {
		Stopped,
        Playing,
        Paused,
    };
    Q_ENUM(Status)
    Status status() const;
	QUrl source() const;
	bool mute() const;
	QString logfile() const;
	int volume() const;

	void setSource(const QUrl& source);
	void setMute(const bool& mute);
	void setLogfile(const QString& logfile);
	void setVolume(const int& volume);

public slots:
	void play();
	void pause();
	void stop();

    bool command(const QVariant& params);
    bool setProperty(const QString& name, const QVariant& value);
    QVariant getProperty(const QString &name, bool *ok=nullptr) const;
	void initCallback();

signals:
    void onUpdate();
	void initFinished();
	void statusChanged();
	void sourceChanged();

private:
	bool inited = false;
	QUrl m_source;
	Status m_status = Stopped;
};

#endif
