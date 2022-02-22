#pragma once

#include <QtQuick/QQuickFramebufferObject>
#include <QtCore/QLoggingCategory>
#include <QtCore/QTimer>

#include "SceneWallpaper.hpp"

Q_DECLARE_LOGGING_CATEGORY(wekdeScene)

class SceneObject : public QQuickItem
{
    Q_OBJECT
	Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
	Q_PROPERTY(QUrl assets READ assets WRITE setAssets)
public:
	QUrl source() const;
	QUrl assets() const;
	void setSource(const QUrl& source);
	void setAssets(const QUrl& assets);
public slots:
    void resizeFb();
signals:
	void sourceChanged();
private:
	QUrl m_source;
	QUrl m_assets;


public:
    static void on_update(void* ctx);

    explicit SceneObject(QQuickItem* parent = nullptr);
    virtual ~SceneObject();

private:
    void setSceneProperty(std::string_view, std::string_view);
    void setSceneProperty(std::string_view, QUrl);
    bool inited = false;
    std::shared_ptr<wallpaper::SceneWallpaper> m_scene {nullptr};

protected:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);
    QTimer timer;
};