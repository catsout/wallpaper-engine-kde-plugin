#pragma once
#include <QtQuick/QQuickFramebufferObject>
#include <QTimer>

class SceneRenderer;

class SceneViewer : public QQuickFramebufferObject
{
    Q_OBJECT
	Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
	Q_PROPERTY(QUrl assets READ assets WRITE setAssets)
	Q_PROPERTY(bool keepAspect READ keepAspect WRITE setKeepAspect NOTIFY keepAspectChanged)

    friend class SceneRenderer;

public:
    SceneViewer(QQuickItem * parent = 0);
    virtual ~SceneViewer();
    virtual Renderer *createRenderer() const;

	QUrl source() const;
	QUrl assets() const;
	bool keepAspect() const;

	void setSource(const QUrl& source);
	void setAssets(const QUrl& assets);
	void setKeepAspect(bool);

	Q_INVOKABLE void setAcceptMouse(bool);
	Q_INVOKABLE void setAcceptHover(bool);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void hoverMoveEvent(QHoverEvent *event);

public slots:
	void play();
	void pause();

signals:
	void sourceChanged();
	void keepAspectChanged();

private:
	QUrl m_source;
	QUrl m_assets;
    QTimer m_updateTimer;
	bool m_keepAspect = false;
	QPointF m_mousePos;
};
