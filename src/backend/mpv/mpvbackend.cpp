#include "mpvbackend.h"

#include <QDir>
#include <QGuiApplication>
#include <QObject>
#include <QOpenGLContext>
#include <QtGlobal>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtQuick/QQuickWindow>
#include <clocale>

#if defined(__linux__) || defined(__FreeBSD__)
//#ifdef ENABLE_X11
#include <QX11Info>  // IWYU pragma: keep
//#endif
#include <qpa/qplatformnativeinterface.h>  // IWYU pragma: keep
#endif

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

    void on_mpv_redraw(void *ctx) {
        MpvObject::on_update(ctx);
    }

    void *get_proc_address_mpv(void *ctx, const char *name) {
        Q_UNUSED(ctx)

        QOpenGLContext *glctx = QOpenGLContext::currentContext();
        if (!glctx)
            return nullptr;

        return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
    }

    int CreateMpvContex(mpv_handle *mpv, mpv_render_context **mpv_gl) {
        mpv_opengl_init_params gl_init_params{get_proc_address_mpv, nullptr, nullptr};
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

}

class MpvRenderer : public QQuickFramebufferObject::Renderer {
public:
    explicit MpvRenderer(MpvObject *new_obj)
        : m_obj{new_obj}, m_mpv_handle{m_obj->mpv}, m_mpv_context{nullptr} {
        mpv_set_wakeup_callback(m_mpv_handle, on_mpv_events, nullptr);
    }

    ~MpvRenderer() override {
        if (m_mpv_context)  // only initialized if something got drawn
        {
            mpv_render_context_free(m_mpv_context);
        }
        mpv_terminate_destroy(m_mpv_handle);
    }

    /*
	 * This function is called when a new FBO is needed.
     * This happens on the initial frame.
	 */
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override {
        QMetaObject::invokeMethod(m_obj, "initCallback");
        return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
    }

    /*
	 * called as a result of QQuickFramebufferObject::update()
	 * called once before the FBO is created
	 * only place when it is safe for the renderer and the item to read and write each others members
	 */
    void synchronize(QQuickFramebufferObject *item) override {
        if (!m_mpv_context && !m_inited) {
            if (CreateMpvContex(m_mpv_handle, &m_mpv_context) >= 0) {
                m_inited = true;
                mpv_render_context_set_update_callback(m_mpv_context, on_mpv_redraw, item);
            }
        }
        item->window()->resetOpenGLState();
    }

    void render() override {
        if (m_inited && m_mpv_context) {
            QOpenGLFramebufferObject *fbo = framebufferObject();
            mpv_opengl_fbo mpfbo{.fbo = static_cast<int>(fbo->handle()), .w = fbo->width(), .h = fbo->height(), .internal_format = 0};
            int flip_y{0};

            mpv_render_param params[] = {
                // Specify the default framebuffer (0) as target. This will
                // render onto the entire screen. If you want to show the video
                // in a smaller rectangle or apply fancy transformations, you'll
                // need to render into a separate FBO and draw it manually.
                {MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
                // Flip rendering (needed due to flipped GL coordinate system).
                {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
                {MPV_RENDER_PARAM_INVALID, nullptr}};
            // See render_gl.h on what OpenGL environment mpv expects, and
            // other API details.
            mpv_render_context_render(m_mpv_context, params);

            // check window if nullptr
            if (m_obj->window() != nullptr) {
                m_obj->window()->resetOpenGLState();
            }
        }
    }

private:
    MpvObject *m_obj;
    bool m_inited{false};

    mpv_handle *m_mpv_handle;
    mpv_render_context *m_mpv_context;
};

MpvObject::MpvObject(QQuickItem *parent)
    : QQuickFramebufferObject(parent), mpv{mpv_create()} {
    // Qt sets the locale in the QGuiApplication constructor, but libmpv
    // requires the LC_NUMERIC category to be set to "C", so change it back.
    std::setlocale(LC_NUMERIC, "C");

    if (!mpv)
        qDebug() << "could not create mpv context";

    mpv_set_option_string(mpv, "terminal", "no");
    mpv_set_option_string(mpv, "msg-level", "all=info");

    if (mpv_initialize(mpv) < 0)
        qDebug() << "could not initialize mpv context";

    mpv_set_option_string(mpv, "hwdec", "auto");
    mpv_set_option_string(mpv, "vo", "libmpv");
    mpv_set_option_string(mpv, "config", "no");
    mpv_set_option_string(mpv, "loop", "yes");

    // Use signal to update at gui thread
    connect(this, &MpvObject::onUpdate, this, &MpvObject::update, Qt::QueuedConnection);
}

MpvObject::~MpvObject() = default;

/*
 * called on the rendering thread while the GUI thread is blocked
 */
QQuickFramebufferObject::Renderer *MpvObject::createRenderer() const {
    window()->setPersistentOpenGLContext(true);
    window()->setPersistentSceneGraph(true);
    return new MpvRenderer(const_cast<MpvObject *>(this));
}

void MpvObject::on_update(void *ctx) {
    MpvObject *self = (MpvObject *)ctx;
    emit self->onUpdate();
}

bool MpvObject::command(const QVariant &params) {
    int errorCode = mpv::qt::get_error(mpv::qt::command(mpv, params));
    return (errorCode >= 0);
}

bool MpvObject::setProperty(const QString &name, const QVariant &value) {
    int errorCode = mpv::qt::get_error(mpv::qt::set_property(mpv, name, value));
    qDebug() << "Setting property" << name << "to" << value;
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
        qWarning() << "Failed to query property: " << name << "code" << errorCode << " result" << result;
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
