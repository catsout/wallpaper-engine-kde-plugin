#pragma once
#include <QQuickItem>
#include <QDBusConnection>

namespace wekde {

class TTYSwitchMonitor : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(bool sleeping READ isSleeping NOTIFY ttySwitch)

public:
    TTYSwitchMonitor(QQuickItem *parent = nullptr);

    bool isSleeping() const { return m_sleeping; }

signals:
    void ttySwitch(bool sleep);

public slots:
    void handlePrepareForSleep(bool sleep);

private:
    bool m_sleeping;
};

}