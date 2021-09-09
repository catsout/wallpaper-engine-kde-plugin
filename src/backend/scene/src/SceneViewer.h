#pragma once
#include <QtQuick/QQuickFramebufferObject>
#include <QTimer>


class SceneRenderer;

class SceneUpdater : public QObject {
    Q_OBJECT
signals:
	void update();
};

class SceneViewer : public QQuickFramebufferObject
{
    Q_OBJECT
	Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
	Q_PROPERTY(QUrl assets READ assets WRITE setAssets)
	Q_PROPERTY(int curFps READ curFps)
	Q_PROPERTY(int fps READ fps WRITE setFps NOTIFY fpsChanged)
	Q_PROPERTY(int fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged)
	Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged)
	Q_PROPERTY(bool muted READ muted WRITE setMuted)

    friend class SceneRenderer;
public:
	enum FillMode {
		STRETCH,
		ASPECTFIT,	
		ASPECTCROP
	};
    Q_ENUM(FillMode)

public:
    explicit SceneViewer(QQuickItem * parent = nullptr);
    ~SceneViewer() override;
    Renderer *createRenderer() const override;

	QUrl source() const;
	QUrl assets() const;
	int curFps() const;
	int fps() const;
	int fillMode() const;
	float volume() const;
	bool muted() const;

	void setSource(const QUrl& source);
	void setAssets(const QUrl& assets);
	void setFps(int);
	void setFillMode(int);
	void setVolume(float);
	void setMuted(bool);

	Q_INVOKABLE void setAcceptMouse(bool);
	Q_INVOKABLE void setAcceptHover(bool);

protected:
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void hoverMoveEvent(QHoverEvent *event) override;

public slots:
	void play();
	void pause();

signals:
	void onUpdate();
	void sourceChanged();
	void fpsChanged();
	void fillModeChanged();
	void volumeChanged();

private:
	QUrl m_source;
	QUrl m_assets;
    QTimer m_updateTimer;
	QPointF m_mousePos;
	bool m_paused;
	int m_fps;
	int m_curFps;
	int m_fillMode;
	float m_volume;
	bool m_muted;
};
