#include "MpvBackend.hpp"


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


#if defined(__linux__) || defined(__FreeBSD__)
//#ifdef ENABLE_X11
#include <QX11Info>  // IWYU pragma: keep
//#endif
#include <qpa/qplatformnativeinterface.h>  // IWYU pragma: keep
#endif

Q_LOGGING_CATEGORY(wekdeMpv, "wekde.mpv")

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

    void *get_proc_address_mpv(void *ctx, const char *name) {
        Q_UNUSED(ctx)

        QOpenGLContext *glctx = QOpenGLContext::currentContext();
        if (!glctx)
            return nullptr;

        return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
    }

    int CreateMpvContex(mpv_handle *mpv, mpv_render_context **mpv_gl) {
        mpv_opengl_init_params gl_init_params{get_proc_address_mpv, nullptr};
        mpv_render_param params[]{
            {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
            {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
            {MPV_RENDER_PARAM_INVALID, nullptr},
            {MPV_RENDER_PARAM_INVALID, nullptr}};

#if defined(__linux__) || defined(__FreeBSD__)
        if (QGuiApplication::platformName().contains("xcb")) {
            params[2].type = MPV_RENDER_PARAM_X11_DISPLAY;
            params[2].data = QX11Info::display();
        }
        if (QGuiApplication::platformName().contains("wayland")) {
            params[2].type = MPV_RENDER_PARAM_WL_DISPLAY;
            auto *native = QGuiApplication::platformNativeInterface();
            params[2].data = native->nativeResourceForWindow("display", nullptr);
        }
#endif
        int code = mpv_render_context_create(mpv_gl, mpv, params);
        return code;
    }

    QSGTexture *createTextureFromGl(uint32_t handle, QSize size, QQuickWindow* window) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        return window->createTextureFromNativeObject(QQuickWindow::NativeObjectTexture, &handle, 0, size);
#else
        return window->createTextureFromId(handle, size);
#endif
    }

}

bool MpvObject::command(const QVariant &params) {
    int errorCode = mpv::qt::get_error(mpv::qt::command(mpv, params));
    return (errorCode >= 0);
}

bool MpvObject::setProperty(const QString &name, const QVariant &value) {
    int errorCode = mpv::qt::get_error(mpv::qt::set_property(mpv, name, value));
    _Q_DEBUG() << "Setting property" << name << "to" << value;
    return (errorCode >= 0);
}

QVariant MpvObject::getProperty(const QString &name, bool *ok) const {
    if (ok)
        *ok = false;

    if (name.isEmpty()) {
        return QVariant();
    }
    QVariant result = mpv::qt::get_property(mpv, name);
    const int errorCode = mpv::qt::get_error(result);
    if (errorCode >= 0) {
        if (ok) {
            *ok = true;
        }
    } else {
        _Q_DEBUG() << "Failed to query property: " << name << "code" << errorCode << " result" << result;
    }
    return result;
}

void MpvObject::initCallback() {
    QUrl temp(m_source.toString());
    m_source.clear();
    inited = true;
    setSource(temp);
    Q_EMIT initFinished();
}

void MpvObject::play() {
    if (status() != Paused)
        return;
    this->setProperty("pause", false);
}

void MpvObject::pause() {
    if (status() != Playing)
        return;
    this->setProperty("pause", true);
}

void MpvObject::stop() {
    if (status() == Stopped)
        return;
    bool result = this->command(QVariantList{"stop"});
    if (result) {
        m_source.clear();
        Q_EMIT sourceChanged();
    }
}

MpvObject::Status MpvObject::status() const {
    const bool stopped =
        getProperty("idle-active").toBool();
    const bool paused = getProperty("pause").toBool();
    return stopped ? Stopped
                   : (paused ? Paused : Playing);
}

QUrl MpvObject::source() const { return m_source; }

bool MpvObject::mute() const {
    return getProperty("mute").toBool();
}

QString MpvObject::logfile() const {
    return getProperty("log-file").toString();
}

int MpvObject::volume() const {
    return getProperty("volume").toInt();
}

void MpvObject::setMute(const bool &mute) {
    setProperty("mute", mute);
}

void MpvObject::setVolume(const int &volume) {
    setProperty("volume", volume);
}

void MpvObject::setLogfile(const QString &logfile) {
    setProperty("log-file", logfile);
}

void MpvObject::setSource(const QUrl &source) {
    if (source.isEmpty()) {
        stop();
        return;
    }
    if (!source.isValid() || (source == m_source)) {
        return;
    }
    if (!inited) {
        m_source = source;
        return;
    }
    bool result = this->command(QVariantList{
        "loadfile",
        source.isLocalFile() ? QDir::toNativeSeparators(source.toLocalFile())
                             : source.url()});
    if (result) {
        m_source = source;
        Q_EMIT sourceChanged();
    }
}


class MpvRender : public QObject {
    Q_OBJECT
public:
    MpvRender(QSize size):m_size(size),m_mpv{mpv_create()} {
        if (!m_mpv)
            _Q_DEBUG() << "could not create mpv context";
        mpv_set_option_string(m_mpv, "terminal", "no");
        mpv_set_option_string(m_mpv, "msg-level", "all=info");
        if (mpv_initialize(m_mpv) < 0)
            _Q_DEBUG() << "could not initialize mpv context";

        mpv_set_option_string(m_mpv, "config", "no");
        mpv_set_option_string(m_mpv, "hwdec", "auto");
        mpv_set_option_string(m_mpv, "vo", "libmpv");
        mpv_set_option_string(m_mpv, "loop", "inf");   

        m_format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        connect(this, &MpvRender::mpvRedraw, this, &MpvRender::renderFrame, Qt::QueuedConnection);

        _Q_DEBUG() << m_size;
    }
    ~MpvRender() = default;    

    QSGTexture* eatFrame(QQuickWindow * window) {
        if(!dirty.exchange(false)) return nullptr;
        presented = ready.exchange(presented);
        FrameBufferParameters* fbo = presented.load();
        if(fbo) {
            QSGTexture* tex = createTextureFromGl(fbo->gltex, fbo->size, window);
            return tex;
        }
        return nullptr;
    }

    mpv_handle* Mpv() { return m_mpv; }

    void markRenderThread(QThread* t) { m_renderThread = t; }

signals:
    void mpvRedraw();
    void newFrame();
    void inited();

public slots:
    void renderFrame() {
        context->makeCurrent(surface);

        if (!m_mpv_context) {
            std::array<AtomicFbo*, 3> atoms {&presented, &ready, &inprogress};
            for(auto& atom:atoms) {
                *atom = new FrameBufferParameters(m_size, m_format);
            }

            //mpv_set_wakeup_callback(m_mpv_handle, on_mpv_events, nullptr);
            if (CreateMpvContex(m_mpv, &m_mpv_context) >= 0) {
                mpv_render_context_set_update_callback(m_mpv_context, on_mpv_redraw, this);
                emit this->inited();
            }
        }
        QOpenGLFramebufferObject* qfbo;
        {
            auto* fbo = inprogress.load();
            if(fbo->qfbo.size() != m_size) {
                delete fbo;
                inprogress = new FrameBufferParameters(m_size, m_format);
                fbo = inprogress.load();
            }
            qfbo = &(fbo->qfbo);
        }
        mpv_opengl_fbo mpfbo{.fbo = static_cast<int>(qfbo->handle()), .w = qfbo->width(), .h = qfbo->height(), .internal_format = 0};
        int flip_y{0};

        mpv_render_param params[] = {
            {MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
            // Flip rendering (needed due to flipped GL coordinate system).
            {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
            {MPV_RENDER_PARAM_INVALID, nullptr}};
        mpv_render_context_render(m_mpv_context, params);
        context->functions()->glFlush();
        inprogress = ready.exchange(inprogress);
        dirty = true;
        emit newFrame();
    }

    void newSize(QSize size) {
        m_size = size;
    }

    void waitShutDown() {
        mpv::qt::command(m_mpv, QVariantList{"stop"});
        QMetaObject::invokeMethod(this, "shutDown", Qt::QueuedConnection);
        if(m_renderThread) m_renderThread->wait();
    }

    void shutDown() {
        _Q_DEBUG() << "destroyed";
        context->makeCurrent(surface);
        {
            std::array<AtomicFbo*, 3> fbos {&presented, &ready, &inprogress};
            for(auto& fb:fbos) {
                delete fb->load();
            }
            if(m_mpv_context) mpv_render_context_free(m_mpv_context);
        }
        context->doneCurrent();
        delete context;

        if(m_mpv) mpv_terminate_destroy(m_mpv);
        // schedule this to be deleted only after we're done cleaning up
        delete surface;
        QThread::currentThread()->exit();
    }

public:
    QOffscreenSurface *surface {nullptr};
    QOpenGLContext *context {nullptr};
    struct FrameBufferParameters {
        FrameBufferParameters(QSize size, QOpenGLFramebufferObjectFormat format):qfbo(size, format),size(size) {
            gltex = qfbo.texture();
        }
        QOpenGLFramebufferObject qfbo;
        uint32_t gltex;
        QSize size;
    };
    typedef std::atomic<FrameBufferParameters*> AtomicFbo;
    AtomicFbo presented {nullptr};
    AtomicFbo ready {nullptr};
    AtomicFbo inprogress {nullptr};

    std::atomic<bool> dirty {false};

private:
    mpv_render_context *m_mpv_context {nullptr};
    mpv_handle *m_mpv {nullptr};
    QOpenGLFramebufferObjectFormat m_format;

    QSize m_size;
    // wait for exit
    QThread *m_renderThread {nullptr};
};

namespace {
    void on_mpv_redraw(void *ctx) {
        emit static_cast<MpvRender*>(ctx)->mpvRedraw();
    }
}

class TextureNode : public QObject, public QSGSimpleTextureNode {
    Q_OBJECT
public:
    typedef std::function<QSGTexture*(QQuickWindow*)> EatFrameOp;
    TextureNode(QQuickWindow *window, EatFrameOp eatFrameOp)
        : m_texture(nullptr)
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

signals:
    void textureInUse();
    void nodeDestroyed();

public slots:
    void newTexture() {
        QSGTexture *newtex = m_eatFrameOp(m_window);
        if(newtex) {
            if(m_texture) delete m_texture;
            m_texture = newtex;
            setTexture(m_texture);
            markDirty(DirtyMaterial);
            emit textureInUse();
        }
    }

private:
    QSGTexture *m_texture;
    EatFrameOp m_eatFrameOp;
    QQuickWindow *m_window;
};

MpvObject::MpvObject(QQuickItem* parent)
    :QQuickItem(parent),
    m_renderThread(nullptr),
    m_mpvRender(nullptr)
{
    setFlag(ItemHasContents, true);
    m_renderThread = new QThread();
    m_mpvRender = new MpvRender({1280, 720});
    mpv = m_mpvRender->Mpv();
    connect(m_mpvRender, &MpvRender::inited, this, &MpvObject::initCallback, Qt::QueuedConnection);

    connect(this, &MpvObject::widthChanged, this, &MpvObject::resizeFb);
    connect(this, &MpvObject::heightChanged, this, &MpvObject::resizeFb);
}

MpvObject::~MpvObject() {}

void MpvObject::resizeFb() {
    QSize size;
    size.setWidth(this->width());
    size.setHeight(this->height());
    QMetaObject::invokeMethod(m_mpvRender, "newSize", Qt::QueuedConnection,
        Q_ARG(QSize, size));
}

void MpvObject::prepareMpv()
{
    m_mpvRender->surface = new QOffscreenSurface();
    m_mpvRender->surface->setFormat(m_mpvRender->context->format());
    m_mpvRender->surface->create();

    m_mpvRender->markRenderThread(m_renderThread);
    m_mpvRender->moveToThread(m_renderThread);

    m_renderThread->start();
    _Q_DEBUG() << "renderThread started";
    update();
}

QSGNode *MpvObject::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    TextureNode *node = static_cast<TextureNode *>(oldNode);

    if (!m_mpvRender->context) {
        _Q_DEBUG() << "setContext ";
        QOpenGLContext *current = window()->openglContext();
        // Some GL implementations requres that the currently bound context is
        // made non-current before we set up sharing, so we doneCurrent here
        // and makeCurrent down below while setting up our own context.
        current->doneCurrent();

        m_mpvRender->context = new QOpenGLContext();
        m_mpvRender->context->setFormat(current->format());
        m_mpvRender->context->setShareContext(current);
        m_mpvRender->context->create();
        m_mpvRender->context->moveToThread(m_renderThread);

        current->makeCurrent(window());

        QMetaObject::invokeMethod(this, "prepareMpv");
        return nullptr;
    }

    if (!node) {
        node = new TextureNode(window(),[this](QQuickWindow * window) {
            return m_mpvRender->eatFrame(window);
        });

        connect(m_mpvRender, &MpvRender::newFrame, window(), &QQuickWindow::update, Qt::QueuedConnection);
        connect(window(), &QQuickWindow::beforeRendering, node, &TextureNode::newTexture, Qt::DirectConnection);

        connect(node, &TextureNode::nodeDestroyed, m_mpvRender, &MpvRender::waitShutDown, Qt::DirectConnection);

        resizeFb();
        on_mpv_redraw(m_mpvRender);
    }

    node->setRect(boundingRect());

    return node;
}

#include "MpvBackend.moc"