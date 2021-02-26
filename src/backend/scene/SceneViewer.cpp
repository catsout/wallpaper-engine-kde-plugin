#include "SceneViewer.h"

#include <stdexcept>
#include <clocale>

#include <QObject>
#include <QtGlobal>
#include <QOpenGLContext>
//#include <QGuiApplication>
#include <QtGui/QOpenGLFramebufferObject>


#include <QDir>
#include <QtQuick/QQuickWindow>
#include <QtQuick/QQuickView>

#include <QLoggingCategory>


namespace
{
static void *get_proc_address(const char *name)
{
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx) return nullptr;
    return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}

}


class SceneRenderer : public QQuickFramebufferObject::Renderer
{
   
public:
    SceneRenderer(SceneViewer* sv)
        : m_viewer{sv} {
    }

    virtual ~SceneRenderer(){}

    // This function is called when a new FBO is needed.
    // This happens on the initial frame.
    QOpenGLFramebufferObject * createFramebufferObject(const QSize &size) {
        return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
    }

	void synchronize(QQuickFramebufferObject *item) {
		if(!framebufferObject()) {
			m_wgl.Init(get_proc_address);
		}
		if(m_source != m_viewer->source() && !m_viewer->assets().isEmpty()) {
			auto assets = QDir::toNativeSeparators(m_viewer->assets().toLocalFile()).toStdString();
			m_wgl.SetAssets(assets);

			m_source = m_viewer->source();
			auto source = QDir::toNativeSeparators(m_source.toLocalFile()).toStdString();
			m_wgl.Load(source);
			m_viewer->window()->resetOpenGLState();
			m_viewer->update();
		}
    }

    void render() {
        QOpenGLFramebufferObject *fbo = framebufferObject();
		m_wgl.Render(fbo->handle(), fbo->width(), fbo->height());
        m_viewer->window()->resetOpenGLState();
    }
private:
	SceneViewer* m_viewer;
	QUrl m_source;
	wallpaper::WallpaperGL m_wgl;
};

SceneViewer::SceneViewer(QQuickItem * parent)
    : QQuickFramebufferObject(parent) {
    int framerate = 35;
     m_updateTimer.setInterval(1000 / framerate);
     connect(&m_updateTimer, &QTimer::timeout, this, [this]() {
         update();
     });
     m_updateTimer.start();
}

SceneViewer::~SceneViewer() {
}

QQuickFramebufferObject::Renderer * SceneViewer::createRenderer() const {
    window()->setPersistentOpenGLContext(true);
    window()->setPersistentSceneGraph(true);
    return new SceneRenderer(const_cast<SceneViewer *>(this));
}

QUrl SceneViewer::source() const { return m_source; }

QUrl SceneViewer::assets() const { return m_assets; }

void SceneViewer::setSource(const QUrl& source) {
	if(source == m_source) return;
	m_source = source;
	update();
	Q_EMIT sourceChanged();
}

void SceneViewer::setAssets(const QUrl& assets) {
	if(m_assets == assets) return;
	m_assets = assets;
};

void SceneViewer::play() {
	m_updateTimer.start();
};

void SceneViewer::pause() {
	m_updateTimer.stop();
};
