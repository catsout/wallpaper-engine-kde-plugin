#pragma once
#include <QQuickItem>
#include <QEvent>
#include <QMouseEvent>

class MouseGrabber : public QQuickItem
{
    Q_OBJECT
	Q_PROPERTY(bool captureMouse READ captureMouse WRITE setCaptureMouse NOTIFY captureMouseChanged)
	Q_PROPERTY(QQuickItem* target READ target WRITE setTarget NOTIFY targetChanged)

public:
    MouseGrabber(QQuickItem *parent = 0);
    virtual ~MouseGrabber() {};

	bool captureMouse() const;
	QQuickItem* target() const;	
	void setCaptureMouse(bool);
	void setTarget(QQuickItem*);

protected:
	void mouseUngrabEvent();
	void mousePressEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent *event);

signals:
	void captureMouseChanged();
	void targetChanged();

private:
	void sendEvent(QEvent*);
	bool m_captureMouse = false;
	QQuickItem* m_target;
};
