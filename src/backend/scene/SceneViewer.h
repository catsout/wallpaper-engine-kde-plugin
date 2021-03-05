#pragma once
#include "wallpaper.h"
#include <QtQuick/QQuickFramebufferObject>
#include <QTimer>
class SceneRenderer;

class SceneViewer : public QQuickFramebufferObject
{
    Q_OBJECT
	Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
	Q_PROPERTY(QUrl assets READ assets WRITE setAssets)
	Q_PROPERTY(bool keepAspect READ keepAspect WRITE setKeepAspect NOTIFY keepAspectChanged)
	Q_PROPERTY(bool captureMouse READ captureMouse NOTIFY captureMouseChanged)

    friend class SceneRenderer;

public:
    SceneViewer(QQuickItem * parent = 0);
    virtual ~SceneViewer();
    virtual Renderer *createRenderer() const;

	QUrl source() const;
	QUrl assets() const;
	bool keepAspect() const;
	bool captureMouse() const;

	void setSource(const QUrl& source);
	void setAssets(const QUrl& assets);
	void setKeepAspect(bool);

	Q_INVOKABLE void setCaptureMouse(bool);

protected:
	void mouseUngrabEvent();
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

public slots:
	void play();
	void pause();

signals:
	void sourceChanged();
	void keepAspectChanged();
	void captureMouseChanged();
private:
	QUrl m_source;
	QUrl m_assets;
    QTimer m_updateTimer;
	bool m_keepAspect = false;
	bool m_captureMouse = false;
	QPointF m_mousePos;
};
