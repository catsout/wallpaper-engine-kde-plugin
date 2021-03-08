#include "MouseGrabber.h"
#include <iostream>
#include <QLoggingCategory>
#include <QCoreApplication>

MouseGrabber::MouseGrabber(QQuickItem * parent):QQuickItem(parent),m_captureMouse(false) {}

bool MouseGrabber::captureMouse() const { return m_captureMouse; }

QQuickItem* MouseGrabber::target() const { return m_target; }

void MouseGrabber::setCaptureMouse(bool value) { 
	if(value == m_captureMouse)
		return;
	m_captureMouse = value;
	if(value) {
		setAcceptedMouseButtons(Qt::AllButtons);
		grabMouse();
	} else {
		setAcceptedMouseButtons(Qt::NoButton);
		ungrabMouse();
	}
	Q_EMIT captureMouseChanged();
}

void MouseGrabber::setTarget(QQuickItem* item) {
	if(item == m_target)
		return;
	m_target = item;
	Q_EMIT targetChanged();
}

void MouseGrabber::mouseUngrabEvent() {
	if(m_captureMouse)
		grabMouse();
}

void MouseGrabber::sendEvent(QEvent* event) {
	if(m_target != nullptr) 
		QCoreApplication::sendEvent(m_target, event);
}

void MouseGrabber::mousePressEvent(QMouseEvent *event) {
	sendEvent(event);
}

void MouseGrabber::mouseMoveEvent(QMouseEvent *event) {
	sendEvent(event);
}

void MouseGrabber::mouseReleaseEvent(QMouseEvent *event) {
	sendEvent(event);
}
