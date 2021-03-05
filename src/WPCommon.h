#pragma once
#include <QObject>
#include <QQuickItem>
#include <QGuiApplication>
#include <QMouseEvent>

class WPCommon : public QObject
{
    Q_OBJECT

public:
    explicit WPCommon(QObject *parent = 0) : QObject(parent) {}

    Q_INVOKABLE void grabMouse(QQuickItem* target) {
        target->grabMouse();
    }

    Q_INVOKABLE void grabMouseWithPress(QQuickItem* target) {
		QMouseEvent press(QEvent::MouseButtonPress, QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        target->grabMouse();
		QGuiApplication::sendEvent(target, &press);
    }

    Q_INVOKABLE void ungrabMouse(QQuickItem* target) {
        target->ungrabMouse();
    }
};
