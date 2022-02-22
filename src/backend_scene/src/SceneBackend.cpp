#include "SceneBackend.h"


#include <QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QDir>
#include <QtCore/QThread>

#include <QtGui/QGuiApplication>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtGui/QOpenGLFunctions>
#include <QtQuick/QQuickWindow>

#include <QtGui/QOffscreenSurface>
#include <QtQuick/QSGSimpleTextureNode> 

#include <clocale>
#include <atomic>
#include <array>
#include <functional>

#include "glExtra.hpp"
#include "SceneWallpaper.hpp"
#include <cstdio>


Q_LOGGING_CATEGORY(wekdeSceen, "wekde.scene")

#define _Q_DEBUG() qCDebug(wekdeMpv)

/// some api tips
/*
 * Assumes the OpenGL context lives on a certain thread
 * All mpv_render_* APIs have to be assumed to implicitly use the OpenGL context, if you pass a mpv_render_context using the OpenGL backend
 *
 */

namespace {
    void on_mpv_events(void *ctx) {
        Q_UNUSED(ctx)
    }

    void on_mpv_redraw(void *ctx);

    void *get_proc_address(const char *name) {
        QOpenGLContext *glctx = QOpenGLContext::currentContext();
        if (!glctx)
            return nullptr;

        return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
    }

    QSGTexture *createTextureFromGl(uint32_t handle, QSize size, QQuickWindow* window) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        return window->createTextureFromNativeObject(QQuickWindow::NativeObjectTexture, &handle, 0, size);
#else
        return window->createTextureFromId(handle, size);
#endif
    }

}

namespace {
    void on_mpv_redraw(void *ctx) {
        //emit static_cast<MpvRender*>(ctx)->mpvRedraw();
    }
}

using sp_scene_t = std::shared_ptr<wallpaper::SceneWallpaper>;

class TextureNode : public QObject, public QSGSimpleTextureNode {
    Q_OBJECT
public:
    typedef std::function<QSGTexture*(QQuickWindow*)> EatFrameOp;
    TextureNode(QQuickWindow *window, sp_scene_t scene, EatFrameOp eatFrameOp)
        : m_texture(nullptr)
        , m_scene(scene)
        , m_eatFrameOp(eatFrameOp)
        , m_window(window)
    {
        // texture node must have a texture, so use the default 0 texture.
        m_texture = createTextureFromGl(0, QSize(1, 1), window);
        setTexture(m_texture);
        setFiltering(QSGTexture::Linear);
        setOwnsTexture(false);
    }

    ~TextureNode() override {
        delete m_texture;
        emit nodeDestroyed();
    }

    bool initGl() {
        return m_glex.init(get_proc_address);
    }

signals:
    void textureInUse();
    void nodeDestroyed();

public slots:
    void newTexture() {
        m_glex.cmd();
        /*
        if(m_ren.inited()) {
            ExHandle* exh = m_ren.exSwapchain().eatFrame();
            if(exh != nullptr) {
                printf("fd: %d, w: %d, h: %d\n", exh->fd, exh->width, exh->height);
                if(texs_map.count(exh) == 0) {
                    int tex = m_glex.genExTexture(*exh);
                    texs_map[exh] = createTextureFromGl(tex, QSize(exh->width, exh->height), m_window); 
                }
                auto newtex = texs_map.at(exh);
                m_texture = newtex;
                setTexture(m_texture);
                markDirty(DirtyMaterial);
                emit textureInUse();
            }
        }
        */
    }

    void vulkan() {
    }

private:
    sp_scene_t m_scene;

    QSGTexture *m_texture;
    EatFrameOp m_eatFrameOp;
    QQuickWindow *m_window;
    GlExtra m_glex;
    std::unordered_map<void*, QSGTexture*> texs_map;
};

SceneObject::SceneObject(QQuickItem* parent)
    :QQuickItem(parent),
     m_scene(std::make_shared<wallpaper::SceneWallpaper>()) {
    setFlag(ItemHasContents, true);
    timer.setInterval(1000);
    timer.start();
    m_scene->init();

    //connect(m_mpvRender, &MpvRender::inited, this, &MpvObject::initCallback, Qt::QueuedConnection);

    //connect(this, &MpvObject::widthChanged, this, &MpvObject::resizeFb);
    //connect(this, &MpvObject::heightChanged, this, &MpvObject::resizeFb);
}

SceneObject::~SceneObject() {}

void SceneObject::resizeFb() {
    QSize size;
    size.setWidth(this->width());
    size.setHeight(this->height());
    //QMetaObject::invokeMethod(m_mpvRender, "newSize", Qt::QueuedConnection,
    //    Q_ARG(QSize, size));
}

QSGNode *SceneObject::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    TextureNode *node = static_cast<TextureNode *>(oldNode);
    if (!node) {
        node = new TextureNode(window(), m_scene,[this](QQuickWindow * window) {
            return (QSGTexture*)nullptr;
        });
        node->initGl();

        //connect(m_mpvRender, &MpvRender::newFrame, window(), &QQuickWindow::update, Qt::QueuedConnection);
        connect(&timer, &QTimer::timeout, node, &TextureNode::vulkan, Qt::QueuedConnection);
        connect(&timer, &QTimer::timeout, window(), &QQuickWindow::update, Qt::QueuedConnection);
        connect(window(), &QQuickWindow::beforeRendering, node, &TextureNode::newTexture, Qt::DirectConnection);
    }

    node->setRect(boundingRect());

    return node;
}

void SceneObject::setSceneProperty(std::string_view name, std::string_view value) {
    if(m_scene == nullptr) {
        return;
    }
    m_scene->setProperty(name, value);
}

void SceneObject::setSceneProperty(std::string_view name, QUrl value) {
    auto str_value = QDir::toNativeSeparators(value.toLocalFile()).toStdString();
    setSceneProperty(name, str_value);
}
// qobject



QUrl SceneObject::source() const { return m_source; }
QUrl SceneObject::assets() const { return m_assets; }

void SceneObject::setSource(const QUrl& source) {
	if(source == m_source) return;
	m_source = source;
    setSceneProperty("source", m_source);
	Q_EMIT sourceChanged();
}

void SceneObject::setAssets(const QUrl& assets) {
	if(m_assets == assets) return;
	m_assets = assets;
    setSceneProperty("assets", m_assets);
}

#include "SceneBackend.moc"