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
#include <array>
#include <functional>
#include <memory>

#if defined(__linux__) || defined(__FreeBSD__)
//#ifdef ENABLE_X11
#    include <QX11Info> // IWYU pragma: keep
//#endif
#    include <qpa/qplatformnativeinterface.h> // IWYU pragma: keep
#endif

Q_LOGGING_CATEGORY(wekdeMpv, "wekde.mpv")

#define _Q_DEBUG() qCDebug(wekdeMpv)

using namespace mpv;

/// some api tips
/*
 * Assumes the OpenGL context lives on a certain thread
 * All mpv_render_* APIs have to be assumed to implicitly use the OpenGL context, if you pass a
 * mpv_render_context using the OpenGL backend
 *
 */

namespace
{
void on_mpv_events(void* ctx) { Q_UNUSED(ctx) }

void on_mpv_redraw(void* ctx);

void* get_proc_address_mpv(void* ctx, const char* name) {
    Q_UNUSED(ctx)

    QOpenGLContext* glctx = QOpenGLContext::currentContext();
    if (! glctx) return nullptr;

    return reinterpret_cast<void*>(glctx->getProcAddress(QByteArray(name)));
}

int CreateMpvContex(mpv_handle* mpv, mpv_render_context** mpv_gl) {
    mpv_opengl_init_params gl_init_params { get_proc_address_mpv, nullptr };
    mpv_render_param       params[] { { MPV_RENDER_PARAM_API_TYPE,
                                  const_cast<char*>(MPV_RENDER_API_TYPE_OPENGL) },
                                { MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params },
                                { MPV_RENDER_PARAM_INVALID, nullptr },
                                { MPV_RENDER_PARAM_INVALID, nullptr } };

#if defined(__linux__) || defined(__FreeBSD__)
    if (QGuiApplication::platformName().contains("xcb")) {
        params[2].type = MPV_RENDER_PARAM_X11_DISPLAY;
        params[2].data = QX11Info::display();
    }
    if (QGuiApplication::platformName().contains("wayland")) {
        params[2].type = MPV_RENDER_PARAM_WL_DISPLAY;
        auto* native   = QGuiApplication::platformNativeInterface();
        params[2].data = native->nativeResourceForWindow("display", nullptr);
    }
#endif
    int code = mpv_render_context_create(mpv_gl, mpv, params);
    return code;
}

QSGTexture* createTextureFromGl(uint32_t handle, QSize size, QQuickWindow* window) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    return window->createTextureFromNativeObject(
        QQuickWindow::NativeObjectTexture, &handle, 0, size);
#else
    return window->createTextureFromId(handle, size);
#endif
}

} // namespace

bool MpvObject::command(const QVariant& params) {
    auto* mpv       = m_mpv;
    int   errorCode = mpv::qt::get_error(mpv::qt::command(mpv, params));
    return (errorCode >= 0);
}

bool MpvObject::setProperty(const QString& name, const QVariant& value) {
    auto* mpv       = m_mpv;
    int   errorCode = mpv::qt::get_error(mpv::qt::set_property(mpv, name, value));
    _Q_DEBUG() << "Setting property" << name << "to" << value;
    return (errorCode >= 0);
}

QVariant MpvObject::getProperty(const QString& name, bool* ok) const {
    auto* mpv = m_mpv;
    if (ok) *ok = false;

    if (name.isEmpty()) {
        return QVariant();
    }
    QVariant  result    = mpv::qt::get_property(mpv, name);
    const int errorCode = mpv::qt::get_error(result);
    if (errorCode >= 0) {
        if (ok) {
            *ok = true;
        }
    } else {
        _Q_DEBUG() << "Failed to query property: " << name << "code" << errorCode << " result"
                   << result;
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
    if (status() != Paused) return;
    this->setProperty("pause", false);
}

void MpvObject::pause() {
    if (status() != Playing) return;
    this->setProperty("pause", true);
}

void MpvObject::stop() {
    if (status() == Stopped) return;
    bool result = this->command(QVariantList { "stop" });
    if (result) {
        m_source.clear();
        Q_EMIT sourceChanged();
    }
}

MpvObject::Status MpvObject::status() const {
    const bool stopped = getProperty("idle-active").toBool();
    const bool paused  = getProperty("pause").toBool();
    return stopped ? Stopped : (paused ? Paused : Playing);
}

QUrl MpvObject::source() const { return m_source; }

bool MpvObject::mute() const { return getProperty("mute").toBool(); }

QString MpvObject::logfile() const { return getProperty("log-file").toString(); }

int MpvObject::volume() const { return getProperty("volume").toInt(); }

void MpvObject::setMute(const bool& mute) { setProperty("mute", mute); }

void MpvObject::setVolume(const int& volume) { setProperty("volume", volume); }

void MpvObject::setLogfile(const QString& logfile) { setProperty("log-file", logfile); }

void MpvObject::setSource(const QUrl& source) {
    if (source.isEmpty()) {
        stop();
        return;
    }
    if (! source.isValid() || (source == m_source)) {
        return;
    }
    if (! inited) {
        m_source = source;
        return;
    }
    bool result = this->command(QVariantList {
        "loadfile",
        source.isLocalFile() ? QDir::toNativeSeparators(source.toLocalFile()) : source.url() });
    if (result) {
        m_source = source;
        Q_EMIT sourceChanged();
    }
}

namespace mpv
{

class MpvRender : public QObject, public QQuickFramebufferObject::Renderer {
    Q_OBJECT
public:
    MpvRender(std::shared_ptr<MpvHandle> mpv, QQuickWindow* win)
        : m_shared_mpv(mpv), m_mpv(mpv.get()->handle), m_window(win) {}

    virtual ~MpvRender() {
        _Q_DEBUG() << "destroyed";
        mpv::qt::command(m_mpv, QVariantList { "stop" });

        if (m_mpv_context) mpv_render_context_free(m_mpv_context);
        m_mpv_context = nullptr;
    }

    bool Dirty() const { return m_dirty.load(); }
    bool setDirty(bool v) { return m_dirty.exchange(v); };

signals:
    void mpvRedraw();
    void inited();

public slots:
    // render thread
    void renderFrame(QOpenGLFramebufferObject* fbo) {
        mpv_opengl_fbo mpfbo { .fbo             = static_cast<int>(fbo->handle()),
                               .w               = fbo->width(),
                               .h               = fbo->height(),
                               .internal_format = 0 };
        int            flip_y { 0 };

        mpv_render_param params[] = {
            { MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo },
            // Flip rendering (needed due to flipped GL coordinate system).
            { MPV_RENDER_PARAM_FLIP_Y, &flip_y },
            { MPV_RENDER_PARAM_INVALID, nullptr }
        };
        mpv_render_context_render(m_mpv_context, params);
    }

    /*
     * This function is called when a new FBO is needed.
     * This happens on the initial frame.
     */
    QOpenGLFramebufferObject* createFramebufferObject(const QSize& size) override {
        // QMetaObject::invokeMethod(m_obj, "initCallback", Qt::QueuedConnection);
        // emit m_updater.inited();
        return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
    }

    /*
     * called as a result of QQuickFramebufferObject::update()
     * called once before the FBO is created
     * only place when it is safe for the renderer and the item to read and write each others
     * members
     */
    void synchronize(QQuickFramebufferObject* item) override {
        if (m_mpv_context == nullptr) {
            // mpv_set_wakeup_callback(m_mpv_handle, on_mpv_events, nullptr);
            if (CreateMpvContex(m_mpv, &m_mpv_context) >= 0) {
                mpv_render_context_set_update_callback(m_mpv_context, on_mpv_redraw, this);
                Q_EMIT this->inited();
            }
        }
        m_window->resetOpenGLState();
    }

    void render() override {
        if (setDirty(false)) {
            QOpenGLFramebufferObject* fbo = framebufferObject();
            renderFrame(fbo);
            m_window->resetOpenGLState();
        }
    }

private:
    mpv_render_context* m_mpv_context { nullptr };
    mpv_handle*         m_mpv { nullptr };
    QQuickWindow*       m_window { nullptr };

    std::shared_ptr<MpvHandle> m_shared_mpv { nullptr };

    std::atomic<bool> m_dirty { false };
};

} // namespace mpv

namespace
{
void on_mpv_redraw(void* ctx) {
    auto* mpv = static_cast<mpv::MpvRender*>(ctx);
    mpv->setDirty(true);
    Q_EMIT mpv->mpvRedraw();
}
} // namespace

MpvObject::MpvObject(QQuickItem* parent)
    : QQuickFramebufferObject(parent), m_shared_mpv(std::make_shared<MpvHandle>(mpv_create())) {
    m_mpv = m_shared_mpv.get()->handle;

    if (! m_mpv) _Q_DEBUG() << "could not create mpv context";
    mpv_set_option_string(m_mpv, "terminal", "no");
    mpv_set_option_string(m_mpv, "msg-level", "all=info");
    if (mpv_initialize(m_mpv) < 0) _Q_DEBUG() << "could not initialize mpv context";

    mpv_set_option_string(m_mpv, "config", "no");
    mpv_set_option_string(m_mpv, "hwdec", "auto");
    mpv_set_option_string(m_mpv, "vo", "libmpv");
    mpv_set_option_string(m_mpv, "loop", "inf");
}

MpvObject::~MpvObject() {}

QQuickFramebufferObject::Renderer* MpvObject::createRenderer() const {
    window()->setPersistentOpenGLContext(true);
    window()->setPersistentSceneGraph(true);

    auto* render = new MpvRender(m_shared_mpv, window());

    // Use Queued signal to update at gui thread
    connect(render, &MpvRender::mpvRedraw, this, &MpvObject::update, Qt::QueuedConnection);
    connect(render, &MpvRender::inited, this, &MpvObject::initCallback, Qt::QueuedConnection);
    return render;
}

#include "MpvBackend.moc"