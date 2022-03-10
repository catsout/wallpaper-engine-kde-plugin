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
#include "SceneWallpaperSurface.hpp"
#include <cstdio>
#include <unistd.h>


Q_LOGGING_CATEGORY(wekdeScene, "wekde.scene")

#define _Q_INFO(fmt, ...) qCInfo(wekdeScene, fmt, __VA_ARGS__)

namespace {
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
        m_init_texture = m_texture;
        setTexture(m_texture);
        setFiltering(QSGTexture::Linear);
        setOwnsTexture(false);
    }

    ~TextureNode() override {
        for(auto& item:texs_map) {
            auto& exh = item.second;
            close(exh.fd);
            m_glex.deleteTexture(exh.gltex);
            delete exh.qsg;
        }
        delete m_init_texture;
        emit nodeDestroyed();
    }

    // only at qt render thread
    bool initGl() {
        return m_glex.init(get_proc_address);
    }
    
    // after gl, can run at any thread
    void initVulkan(uint16_t w, uint16_t h) {
        wallpaper::RenderInitInfo info;
        info.enable_valid_layer = true;
        info.offscreen = true;
        info.uuid = m_glex.uuid();
        info.width = w;
        info.height = h;
        info.redraw_callback = [this]() {
            emit this->redraw();
        };
        // this send to looper, not in this thread
        m_scene->initVulkan(info);
    }
signals:
    void textureInUse();
    void nodeDestroyed();
    void redraw();

public slots:
    void newTexture() {
        if(!m_scene->inited() || m_scene->exSwapchain() == nullptr) return;

        wallpaper::ExHandle* exh = m_scene->exSwapchain()->eatFrame();
        if(exh != nullptr) {
            int id = exh->id();
            if(texs_map.count(id) == 0) {
                _Q_INFO("receive external texture(%dx%d) from fd: %d", exh->width, exh->height, exh->fd);
                uint gltex = m_glex.genExTexture(*exh);
                ExTex ex_tex;
                ex_tex.fd = exh->fd;
                ex_tex.gltex = gltex;
                ex_tex.qsg = createTextureFromGl(gltex, QSize(exh->width, exh->height), m_window); 
                texs_map[id] = ex_tex;
            }
            auto& newtex = texs_map.at(id);
            m_texture = newtex.qsg;

            setTexture(m_texture);
            markDirty(DirtyMaterial);
            emit textureInUse();
        }
    }

private:
    sp_scene_t m_scene;

    QSGTexture *m_init_texture;
    QSGTexture *m_texture;
    EatFrameOp m_eatFrameOp;
    QQuickWindow *m_window;
    GlExtra m_glex;

    std::unordered_map<int, uint> gltexs_map;
    struct ExTex {
        int fd;
        uint gltex;
        QSGTexture* qsg;
    };
    std::unordered_map<int, ExTex> texs_map;
};

SceneObject::SceneObject(QQuickItem* parent)
    :QQuickItem(parent),
     m_scene(std::make_shared<wallpaper::SceneWallpaper>()) {
    setFlag(ItemHasContents, true);
    m_scene->init();
}

SceneObject::~SceneObject() {}

void SceneObject::resizeFb() {
    QSize size;
    size.setWidth(this->width());
    size.setHeight(this->height());
}

QSGNode *SceneObject::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    TextureNode *node = static_cast<TextureNode *>(oldNode);
    if (!node) {
        node = new TextureNode(window(), m_scene,[this](QQuickWindow * window) {
            return (QSGTexture*)nullptr;
        });
        node->initGl();
        node->initVulkan(width(), height());

        
        connect(node, &TextureNode::redraw, window(), &QQuickWindow::update, Qt::QueuedConnection);
        connect(window(), &QQuickWindow::beforeRendering, node, &TextureNode::newTexture, Qt::DirectConnection);
    }

    node->setRect(boundingRect());
    return node;
}

#define SET_PROPERTY(type, name, value)   \
m_scene->setProperty##type(name, value);  

void SceneObject::setScenePropertyQurl(std::string_view name, QUrl value) {
    auto str_value = QDir::toNativeSeparators(value.toLocalFile()).toStdString();
    SET_PROPERTY(String, name, str_value);
}
// qobject


QUrl SceneObject::source() const { return m_source; }
QUrl SceneObject::assets() const { return m_assets; }

int SceneObject::fps() const { return m_fps; }
int SceneObject::fillMode() const { return m_fillMode; }
float SceneObject::volume() const { return m_volume; }
bool SceneObject::muted() const { return m_muted; }

void SceneObject::setSource(const QUrl& source) {
	if(source == m_source) return;
	m_source = source;
    setScenePropertyQurl("source", m_source);
	Q_EMIT sourceChanged();
}

void SceneObject::setAssets(const QUrl& assets) {
	if(m_assets == assets) return;
	m_assets = assets;
    setScenePropertyQurl("assets", m_assets);
}

void SceneObject::setFps(int value) {
	if(m_fps == value) return;
	m_fps = value;
    SET_PROPERTY(Int32, "fps", value);
	Q_EMIT fpsChanged();
}
void SceneObject::setFillMode(int value) {
	if(m_fillMode == value) return;
	m_fillMode = value;
    SET_PROPERTY(Int32, "fillmode", value);
	Q_EMIT fillModeChanged();
}
void SceneObject::setVolume(float value) {
	if(m_volume == value) return;
	m_volume = value;
    SET_PROPERTY(Float, "volume", value);
	Q_EMIT volumeChanged();
}
void SceneObject::setMuted(bool value) {
	if(m_muted == value) return;
	m_muted = value;
    SET_PROPERTY(Bool, "muted", value);
}


#include "SceneBackend.moc"