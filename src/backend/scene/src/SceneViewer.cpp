#include "SceneViewer.h"


#include <QObject>
#include <QtGlobal>
#include <QOpenGLContext>
#include <QtGui/QOpenGLFramebufferObject>

#include <QFileInfo>
#include <QDir>
#include <QtQuick/QQuickWindow>

#include <QLoggingCategory>
#include "wallpaper.h"
#include "Type.h"


namespace
{
void *get_proc_address(const char *name)
{
	QOpenGLContext *glctx = QOpenGLContext::currentContext();
	if (!glctx) return nullptr;
	return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}

wallpaper::FillMode ToWPFillMode(int fillMode) {
	switch ((SceneViewer::FillMode)fillMode)
	{
	case SceneViewer::FillMode::STRETCH:
		return wallpaper::FillMode::STRETCH;
	case SceneViewer::FillMode::ASPECTFIT:
		return wallpaper::FillMode::ASPECTFIT;
	case SceneViewer::FillMode::ASPECTCROP:
	default:
		return wallpaper::FillMode::ASPECTCROP;
	}
}

}

class SceneRenderer : public QQuickFramebufferObject::Renderer
{
public:
	explicit SceneRenderer(QQuickWindow* window):m_window(window) {};

	~SceneRenderer() override = default;

	// This function is called when a new FBO is needed.
	// This happens on the initial frame.
	QOpenGLFramebufferObject * createFramebufferObject(const QSize &size) override {
		auto* fbo = QQuickFramebufferObject::Renderer::createFramebufferObject(size);
		if(m_wgl.Loaded()) {
			m_wgl.SetDefaultFbo(fbo->handle(), fbo->width(), fbo->height());
			fboNotSet = false;
		}
		else fboNotSet = true;
		return fbo;
	}

	void synchronize(QQuickFramebufferObject *item) override {
		SceneViewer* viewer = static_cast<SceneViewer*>(item);
		if(!framebufferObject()) {
			m_wgl.Init(get_proc_address);
			m_wgl.SetUpdateCallback([this]() {
				emit m_updater.update();
			});
			m_wgl.SetFillMode(ToWPFillMode(m_fillMode));
		}

		// operator requre loaded
		if(m_wgl.Loaded()) {
			if(m_fillMode != (SceneViewer::FillMode)viewer->m_fillMode) {
				m_fillMode = (SceneViewer::FillMode)viewer->m_fillMode;
				m_wgl.SetFillMode(ToWPFillMode(m_fillMode));
			}
			if(fboNotSet) {
				QOpenGLFramebufferObject *fbo = framebufferObject();
				m_wgl.SetDefaultFbo(fbo->handle(), fbo->width(), fbo->height());
				fboNotSet = false;
			}
		}

		if(m_mousePos != viewer->m_mousePos) {
			m_mousePos = viewer->m_mousePos;
			m_wgl.SetMousePos(m_mousePos.x(), m_mousePos.y());
		}
		if(m_paused != viewer->m_paused) {
			m_paused = viewer->m_paused;
			if(m_paused)
				m_wgl.Stop();
			else
				m_wgl.Start();
		}
		if(m_volume != viewer->m_volume) {
			m_volume = viewer->m_volume;
			m_wgl.SetVolume(m_volume);
		}
		if(m_muted != viewer->m_muted) {
			m_muted = viewer->m_muted;
			m_wgl.SetMuted(m_muted);
		}
		if(viewer->fps() != m_wgl.Fps()) {
			m_wgl.SetFps(viewer->fps());
		}


		// load source last
		if(m_source != viewer->source() && !viewer->assets().isEmpty()) {
			auto assets = QDir::toNativeSeparators(viewer->assets().toLocalFile()).toStdString();
			m_wgl.SetAssets(assets);

			m_source = viewer->source();
			auto pkgdir = QDir::toNativeSeparators(m_source.adjusted(QUrl::RemoveFilename).toLocalFile()).toStdString();
			auto entry = QFileInfo(m_source.fileName()).completeBaseName().toStdString();
			m_wgl.LoadPkg(pkgdir, entry);
			m_window->resetOpenGLState();
		}

		viewer->m_curFps = m_wgl.CurrentFps();
	}

	void render() override {
		m_wgl.Render();
		m_window->resetOpenGLState();
	}

	SceneUpdater* Updater() { return &m_updater; }
private:
	QQuickWindow* m_window;
	SceneUpdater m_updater;
	QUrl m_source;
	wallpaper::WallpaperGL m_wgl;
	bool fboNotSet {true};
	QPointF m_mousePos;
	bool m_paused {false};
	float m_volume {1.0f};
	bool m_muted;
	SceneViewer::FillMode m_fillMode {SceneViewer::FillMode::ASPECTCROP};
};

SceneViewer::SceneViewer(QQuickItem * parent):QQuickFramebufferObject(parent),
		m_mousePos(0,0),
		m_fps(15),
		m_paused(false),
		m_curFps(0),
		m_fillMode(FillMode::ASPECTCROP),
		m_volume(1.0f),
		m_muted(false) {
}

SceneViewer::~SceneViewer() {
}

QQuickFramebufferObject::Renderer * SceneViewer::createRenderer() const {
	window()->setPersistentOpenGLContext(true);
	window()->setPersistentSceneGraph(true);
	auto* render =  new SceneRenderer(window());
	connect(render->Updater(), &SceneUpdater::update, this, &SceneViewer::update, Qt::QueuedConnection);
	return render;
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
int SceneViewer::fillMode() const { return m_fillMode; }
float SceneViewer::volume() const { return m_volume; }
bool SceneViewer::muted() const { return m_muted; }

void SceneViewer::setSource(const QUrl& source) {
	if(source == m_source) return;
	m_source = source;
	update();
	Q_EMIT sourceChanged();
}

void SceneViewer::setAssets(const QUrl& assets) {
	if(m_assets == assets) return;
	m_assets = assets;
}

void SceneViewer::setFps(int value) {
	if(m_fps == value) return;
	m_fps = value;
	Q_EMIT fpsChanged();
}
void SceneViewer::setFillMode(int value) {
	if(m_fillMode == value) return;
	m_fillMode = value;
	Q_EMIT fillModeChanged();
}
void SceneViewer::setVolume(float value) {
	if(m_volume == value) return;
	m_volume = value;
	Q_EMIT volumeChanged();
}
void SceneViewer::setMuted(bool value) {
	if(m_muted == value) return;
	m_muted = value;
}

void SceneViewer::play() {
	m_paused = false;
	update();
}

void SceneViewer::pause() {
	m_paused = true;
}
