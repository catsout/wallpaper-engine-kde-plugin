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

    friend class SceneRenderer;

public:
    SceneViewer(QQuickItem * parent = 0);
    virtual ~SceneViewer();
    virtual Renderer *createRenderer() const;

	QUrl source() const;
	QUrl assets() const;

	void setSource(const QUrl& source);
	void setAssets(const QUrl& assets);

public slots:
	void play();
	void pause();

signals:
	void sourceChanged();
private:
	QUrl m_source;
	QUrl m_assets;
    QTimer m_updateTimer;
};
