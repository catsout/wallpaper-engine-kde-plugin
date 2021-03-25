#include "SceneViewer.h"
#include "wallpaper.h"

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
			m_wgl.SetUpdateCallback(std::bind(&SceneRenderer::updateViewer, this));
			// Set m_wgl visible to viewer, only for stop when destroy
			m_viewer->m_wgl = &m_wgl;
		}
		if(m_mousePos != m_viewer->m_mousePos) {
			m_mousePos = m_viewer->m_mousePos;
			m_wgl.SetMousePos(m_mousePos.x(), m_mousePos.y());
		}
		if(m_source != m_viewer->source() && !m_viewer->assets().isEmpty()) {
			auto assets = QDir::toNativeSeparators(m_viewer->assets().toLocalFile()).toStdString();
			m_wgl.SetAssets(assets);

			m_source = m_viewer->source();
			auto source = QDir::toNativeSeparators(m_source.toLocalFile()).toStdString();
			m_wgl.Load(source);
			m_viewer->window()->resetOpenGLState();
		}
		if(m_paused != m_viewer->m_paused) {
			m_paused = m_viewer->m_paused;
			if(m_paused)
				m_wgl.Stop();
			else
				m_wgl.Start();
		}
		if(m_viewer->fps() != m_wgl.Fps()) {
			m_wgl.SetFps(m_viewer->fps());
		}

		m_viewer->m_curFps = m_wgl.CurrentFps();
    }

    void render() {
        QOpenGLFramebufferObject *fbo = framebufferObject();
		m_wgl.Render(fbo->handle(), fbo->width(), fbo->height());
		m_viewer->window()->resetOpenGLState();
    }

	void updateViewer() {
		emit m_viewer->onUpdate();
	}
private:
	SceneViewer* m_viewer;
	QUrl m_source;
	wallpaper::WallpaperGL m_wgl;
	QPointF m_mousePos;
	bool m_paused;
};

SceneViewer::SceneViewer(QQuickItem * parent):QQuickFramebufferObject(parent),
		m_mousePos(0,0),
		m_fps(15),
		m_paused(false),
		m_wgl(nullptr) {
    connect(this, &SceneViewer::onUpdate, this, &SceneViewer::update, Qt::QueuedConnection);
}

SceneViewer::~SceneViewer() {
	// make sure stop m_wgl before destroy
	if(m_wgl != nullptr)
		static_cast<wallpaper::WallpaperGL*>(m_wgl)->Stop();
}

QQuickFramebufferObject::Renderer * SceneViewer::createRenderer() const {
    window()->setPersistentOpenGLContext(true);
    window()->setPersistentSceneGraph(true);
    return new SceneRenderer(const_cast<SceneViewer *>(this));
}

void SceneViewer::setAcceptMouse(bool value) {
	if(value)
		setAcceptedMouseButtons(Qt::LeftButton);
	else
		setAcceptedMouseButtons(Qt::NoButton);
}

void SceneViewer::setAcceptHover(bool value) {
	setAcceptHoverEvents(value);
}

void SceneViewer::mousePressEvent(QMouseEvent *event) {
}

void SceneViewer::mouseMoveEvent(QMouseEvent *event) {
	m_mousePos = event->localPos();
}

void SceneViewer::hoverMoveEvent(QHoverEvent *event) {
	m_mousePos = event->posF();
}

QUrl SceneViewer::source() const { return m_source; }

QUrl SceneViewer::assets() const { return m_assets; }


int SceneViewer::curFps() const { return m_curFps; }

int SceneViewer::fps() const { return m_fps; }

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

void SceneViewer::setFps(int value) {
	if(m_fps == value) return;
	m_fps = value;
	Q_EMIT fpsChanged();
};


void SceneViewer::play() {
	m_paused = false;
	update();
};

void SceneViewer::pause() {
	m_paused = true;
};
