#include "TTYSwitchMonitor.hpp"
#include <QDebug>

using namespace wekde;

TTYSwitchMonitor::TTYSwitchMonitor(QQuickItem *parent)
    : QQuickItem(parent), m_sleeping(false) {
    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.isConnected()) {
        qFatal("Cannot connect to the D-Bus system bus");
        return;
    }

    bool connected = systemBus.connect(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager",
        "PrepareForSleep",
        this,
        SLOT(handlePrepareForSleep(bool))
    );

    if (!connected) {
        qFatal("Failed to connect to PrepareForSleep signal");
    }
}

void TTYSwitchMonitor::handlePrepareForSleep(bool sleep) {
    if (m_sleeping != sleep) {
        m_sleeping = sleep;
        emit ttySwitch(sleep);
    }
}
