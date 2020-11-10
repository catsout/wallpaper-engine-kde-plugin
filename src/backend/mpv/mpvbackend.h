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

    mpv_handle *mpv;
    mpv_render_context *mpv_gl;

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

	void setSource(const QUrl& source);
	void setMute(const bool& mute);
	void setLogfile(const QString& logfile);

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

private slots:
    void doUpdate();

private:
	bool inited = false;
	QUrl m_source;
	Status m_status = Stopped;
};

#endif
