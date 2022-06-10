#include "MouseGrabber.hpp"
#include <iostream>
#include <QLoggingCategory>
#include <QCoreApplication>

using namespace wekde;

MouseGrabber::MouseGrabber(QQuickItem* parent): QQuickItem(parent) {
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
}

bool MouseGrabber::forceCapture() const { return m_forceCapture; }

QQuickItem* MouseGrabber::target() const { return m_target; }

void MouseGrabber::setForceCapture(bool value) {
    if (value == m_forceCapture) return;
    m_forceCapture = value;
    if (value) {
        grabMouse();
    } else {
        ungrabMouse();
    }
    Q_EMIT forceCaptureChanged();
}

void MouseGrabber::setTarget(QQuickItem* item) {
    if (item == m_target) return;
    m_target = item;
    if (m_target) m_target->setAcceptedMouseButtons(Qt::LeftButton);
    Q_EMIT targetChanged();
}

void MouseGrabber::mouseUngrabEvent() {
    if (m_forceCapture) grabMouse();
}

void MouseGrabber::sendEvent(QObject* target, QEvent* event) {
    QCoreApplication::sendEvent(target, event);
}

void MouseGrabber::sendMouseEvent(QMouseEvent* event) {
    if (m_target) {
        QMouseEvent* temp = new QMouseEvent(event->type(),
                                            mapToItem(m_target, event->localPos()),
                                            event->screenPos(),
                                            event->button(),
                                            event->buttons(),
                                            event->modifiers());
        QCoreApplication::postEvent(m_target, temp);
    }
}

void MouseGrabber::sendHoverEvent(QHoverEvent* event) {
    if (m_target) {
        QHoverEvent* temp = new QHoverEvent(event->type(),
                                            mapToItem(m_target, event->posF()),
                                            mapToItem(m_target, event->oldPosF()),
                                            event->modifiers());
        QCoreApplication::postEvent(m_target, temp);
    }
}

void MouseGrabber::mousePressEvent(QMouseEvent* event) {
    sendMouseEvent(event);
    // need accept press to receive release
    // this break long press on desktop
    event->accept();
}

void MouseGrabber::mouseMoveEvent(QMouseEvent* event) {
    sendMouseEvent(event);
    event->ignore();
}

void MouseGrabber::mouseReleaseEvent(QMouseEvent* event) {
    sendMouseEvent(event);
    event->ignore();
}

void MouseGrabber::mouseDoubleClickEvent(QMouseEvent* event) {
    sendMouseEvent(event);
    event->ignore();
}

void MouseGrabber::hoverMoveEvent(QHoverEvent* event) {
    sendHoverEvent(event);
    event->ignore();
}
