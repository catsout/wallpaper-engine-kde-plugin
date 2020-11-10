/*
 *  Copyright 2020 catsout  <outl941@163.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

#include "mpvbackend.h"

#include <stdexcept>
#include <clocale>

#include <QObject>
#include <QtGlobal>
#include <QOpenGLContext>
#include <QGuiApplication>
#include <QDir> 
#include <QtGui/QOpenGLFramebufferObject>

#include <QtQuick/QQuickWindow>
#include <QtQuick/QQuickView>


#if defined(__linux__) || defined(__FreeBSD__)
//#ifdef ENABLE_X11
#include <QX11Info> // IWYU pragma: keep
//#endif
#include <qpa/qplatformnativeinterface.h> // IWYU pragma: keep
#endif


namespace
{
void on_mpv_events(void *ctx)
{
    Q_UNUSED(ctx)
}

void on_mpv_redraw(void *ctx)
{
    MpvObject::on_update(ctx);
}

static void *get_proc_address_mpv(void *ctx, const char *name)
{
    Q_UNUSED(ctx)

    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx) return nullptr;

    return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}

}

class MpvRenderer : public QQuickFramebufferObject::Renderer
{
    MpvObject *obj;

public:
    MpvRenderer(MpvObject *new_obj)
        : obj{new_obj}
    {
        mpv_set_wakeup_callback(obj->mpv, on_mpv_events, nullptr);
    }

    virtual ~MpvRenderer()
    {}

    // This function is called when a new FBO is needed.
    // This happens on the initial frame.
    QOpenGLFramebufferObject * createFramebufferObject(const QSize &size)
    {
        // init mpv_gl:
        if (!obj->mpv_gl)
        {
            mpv_opengl_init_params gl_init_params{get_proc_address_mpv, nullptr, nullptr};
            mpv_render_param params[]{
                {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
                {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
                {MPV_RENDER_PARAM_INVALID, nullptr},
				{MPV_RENDER_PARAM_INVALID, nullptr}
            };

#if defined(__linux__) || defined(__FreeBSD__)
//#ifdef ENABLE_X11
            if (QGuiApplication::platformName().contains("xcb")) {
                params[2].type = MPV_RENDER_PARAM_X11_DISPLAY;
                params[2].data = QX11Info::display();
            }
//#endif
            if (QGuiApplication::platformName().contains("wayland")) {
                params[2].type = MPV_RENDER_PARAM_WL_DISPLAY;
                auto* native = QGuiApplication::platformNativeInterface();
                params[2].data = native->nativeResourceForWindow("display", NULL);
            }
#endif


            if (mpv_render_context_create(&obj->mpv_gl, obj->mpv, params) < 0)
                throw std::runtime_error("failed to initialize mpv GL context");
            mpv_render_context_set_update_callback(obj->mpv_gl, on_mpv_redraw, obj);
        }
		QMetaObject::invokeMethod(obj, "initCallback");
        return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
    }

    void render()
    {
        obj->window()->resetOpenGLState();

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
            {MPV_RENDER_PARAM_INVALID, nullptr}
        };
        // See render_gl.h on what OpenGL environment mpv expects, and
        // other API details.
        mpv_render_context_render(obj->mpv_gl, params);

        obj->window()->resetOpenGLState();
    }
};

MpvObject::MpvObject(QQuickItem * parent)
    : QQuickFramebufferObject(parent), mpv{mpv_create()}, mpv_gl(nullptr)
{
	// Qt sets the locale in the QGuiApplication constructor, but libmpv
    // requires the LC_NUMERIC category to be set to "C", so change it back.
	std::setlocale(LC_NUMERIC, "C");
    
	if (!mpv)
        throw std::runtime_error("could not create mpv context");

    mpv_set_option_string(mpv, "terminal", "no");
    mpv_set_option_string(mpv, "msg-level", "all=v");

    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error("could not initialize mpv context");

    // Request hw decoding, just for testing.
    mpv::qt::set_option_variant(mpv, "hwdec", "auto");
	mpv_set_option_string(mpv, "vo", "libmpv");
	mpv_set_option_string(mpv, "config", "no");
	mpv_set_option_string(mpv, "loop", "yes");

    connect(this, &MpvObject::onUpdate, this, &MpvObject::doUpdate,
           Qt::QueuedConnection);
}

MpvObject::~MpvObject()
{
    if (mpv_gl) // only initialized if something got drawn
    {
        mpv_render_context_free(mpv_gl);
    }

    mpv_terminate_destroy(mpv);
}

void MpvObject::on_update(void *ctx)
{
    MpvObject *self = (MpvObject *)ctx;
    emit self->onUpdate();
}

// connected to onUpdate(); signal makes sure it runs on the GUI thread
void MpvObject::doUpdate()
{
    update();
}

bool MpvObject::command(const QVariant& params)
{
	int errorCode = 0;
    errorCode = mpv::qt::get_error(mpv::qt::command(mpv, params));
	return (errorCode >= 0);
}

bool MpvObject::setProperty(const QString& name, const QVariant& value)
{
	int errorCode = 0;
	qDebug() << "Setting property" << name << "to" << value;
    errorCode = mpv::qt::get_error(mpv::qt::set_property(mpv, name, value));
	return (errorCode >= 0);
}

QVariant MpvObject::getProperty(const QString &name, bool *ok) const {
    if (ok)
        *ok = false;
    
    if (name.isEmpty()) {
        return QVariant();
    }
    const QVariant result = mpv::qt::get_property(mpv, name);
    const int errorCode = mpv::qt::get_error(result);
	if(errorCode >= 0)
	{
		if(ok){
			*ok = true;
		}
	}
	else {
		qWarning() << "Failed to query property: " << name << "code" << errorCode << " result" << result;
	}
	return result;
}

QQuickFramebufferObject::Renderer *MpvObject::createRenderer() const
{
    window()->setPersistentOpenGLContext(true);
    window()->setPersistentSceneGraph(true);
    return new MpvRenderer(const_cast<MpvObject *>(this));
}

void MpvObject::initCallback()
{
	QUrl temp(m_source.toString());
	m_source.clear();
	inited = true;
	setSource(temp);
	Q_EMIT initFinished();
}

//
void MpvObject::play()
{
	if(status() != Paused) return;
	bool result = this->setProperty(QString::fromUtf8("pause"),false);
}

void MpvObject::pause()
{	
	if(status() != Playing) return;
	//qDebug() << "status:" << m_status;	
	bool result = this->setProperty(QString::fromUtf8("pause"),true);
}

void MpvObject::stop()
{
	if(status() == Stopped) return;
	bool result = this->command(QVariantList{QString::fromUtf8("stop")});
	if(result){
		m_source.clear();
		Q_EMIT sourceChanged();
	}
}

MpvObject::Status MpvObject::status() const { 
	const bool stopped =
		getProperty(QString::fromUtf8("idle-active")).toBool();
	const bool paused = getProperty(QString::fromUtf8("pause")).toBool();
	return stopped ? Stopped
		: (paused ? Paused : Playing);
}

QUrl MpvObject::source() const { return m_source; }

bool MpvObject::mute() const {
    return getProperty(QString::fromUtf8("mute")).toBool();
}

QString MpvObject::logfile() const {
    return getProperty(QString::fromUtf8("log-file")).toString();
}

int MpvObject::volume() const {
    return getProperty(QString::fromUtf8("volume")).toInt();
}

void MpvObject::setMute( const bool& mute){
	setProperty(QString::fromUtf8("mute"), mute);
}

void MpvObject::setVolume( const int& volume){
	setProperty(QString::fromUtf8("volume"), volume);
}

void MpvObject::setLogfile( const QString& logfile){
	setProperty(QString::fromUtf8("log-file"), logfile);
}

void MpvObject::setSource( const QUrl& source ) 
{
    if (source.isEmpty()) {
        stop();
        return;
    }
    if (!source.isValid() || (source == m_source)) {
        return;
    }
	if (!inited)
	{
		m_source = source;
		return;
	}
	bool result = this->command(QVariantList{
        QString::fromUtf8("loadfile"),
        source.isLocalFile() ? QDir::toNativeSeparators(source.toLocalFile())
                             : source.url()});
	if(result){
		m_source = source;
		Q_EMIT sourceChanged();
	}
}	
