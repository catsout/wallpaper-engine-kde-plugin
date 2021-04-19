#pragma once
#include <QQuickItem>
#include <QEvent>
#include <QMouseEvent>
#include <QHoverEvent>
#include <QPointer>

class MouseGrabber : public QQuickItem
{
    Q_OBJECT
	Q_PROPERTY(bool forceCapture READ forceCapture WRITE setForceCapture NOTIFY forceCaptureChanged)
	Q_PROPERTY(QQuickItem* target READ target WRITE setTarget NOTIFY targetChanged)

public:
    MouseGrabber(QQuickItem *parent = 0);
    virtual ~MouseGrabber() {};

	bool forceCapture() const;
	QQuickItem* target() const;	

	void setForceCapture(bool);
	void setTarget(QQuickItem*);

	Q_INVOKABLE void sendEvent(QObject*, QEvent*);

protected:
	void mouseUngrabEvent();
	void mousePressEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void hoverMoveEvent(QHoverEvent *);

signals:
	void forceCaptureChanged();
	void targetChanged();

private:
    void sendMouseEvent(QMouseEvent*);
    void sendHoverEvent(QHoverEvent*);
	bool m_forceCapture {false};
    QPointer<QQuickItem> m_target {nullptr};
};
