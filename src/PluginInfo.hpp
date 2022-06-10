#pragma once
#include <QObject>

namespace wekde
{

class PluginInfo : public QObject {
    Q_OBJECT
    // Q_PROPERTY(READ WRITE NOTIFY)
    Q_PROPERTY(QUrl cache_path READ cache_path NOTIFY cache_pathChanged)
    Q_PROPERTY(QUrl version READ version NOTIFY versionChanged)

public:
    PluginInfo(QObject* parent = nullptr);
    virtual ~PluginInfo();

    QUrl cache_path() const;

    QString version() const { return "0.5.3"; };

protected:
signals:
    void cache_pathChanged();
    void versionChanged();
};
} // namespace wekde
