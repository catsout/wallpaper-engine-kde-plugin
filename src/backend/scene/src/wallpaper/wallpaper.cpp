#include "wallpaper.h"
#include "Log.h"

#include "FpsCounter.h"
#include "FrameTimer.h"

#include "Scene.h"
#include "WPSceneParser.h"
#include "GLGraphicManager.h"

#include <chrono>
#include <thread>

typedef wallpaper::fs::file_node file_node;
using namespace wallpaper;

file_node wallpaper::WallpaperGL::m_pkgfs = file_node();

class WallpaperGL::impl {
public:
	impl() = default;
	FpsCounter fpsCounter;
	FrameTimer frameTimer;
	WPSceneParser parser;
	GLGraphicManager gm;

	std::chrono::time_point<std::chrono::steady_clock> timer {std::chrono::steady_clock::now()};
	std::unique_ptr<Scene> scene;
	float fboW {1920.0f};
	float fboH {1080.0f};
};

WallpaperGL::WallpaperGL():m_mousePos({0,0}),m_aspect(16.0f/9.0f),pImpl(std::make_unique<impl>())
 {}

WallpaperGL::~WallpaperGL() {
	Clear();
}

bool WallpaperGL::Init(void *get_proc_address(const char *)) {
	m_inited = pImpl->gm.Initialize(get_proc_address);
	return m_inited;
}

void WallpaperGL::Load(const std::string& pkg_path) {
	if(!m_inited || pkg_path == m_pkgPath) return;
	m_loaded = false;
	if(!m_pkgPath.empty()) {
		Clear();
	}
	//load pkgfile
	m_pkgfs = file_node();
	if(fs::ReadPkgToNode(m_pkgfs,pkg_path) == -1) {
		LOG_ERROR("Can't load " + pkg_path);
		return;
	}
	m_pkgPath = pkg_path;
	//fs::PrintFileTree(m_pkgfs, 100);
	std::string scene_src;
	if(wallpaper::fs::IsFileInNode(m_pkgfs, "scene.json"))
		scene_src = wallpaper::fs::GetContent(m_pkgfs, "scene.json");
	else if(wallpaper::fs::IsFileInNode(m_pkgfs, "gifscene.json"))
		scene_src = wallpaper::fs::GetContent(m_pkgfs, "gifscene.json");
	if(scene_src.empty()) {
		LOG_ERROR("Not supported scene type");
		return;
	}
	pImpl->scene = pImpl->parser.Parse(scene_src);	
	if(pImpl->scene) {
		pImpl->gm.InitializeScene(pImpl->scene.get());
	} else return;
	m_loaded = true;

	// required as camera is reloaded
	SetFillMode(m_fillMode);
}

void WallpaperGL::Render() {
	if(!m_inited || !m_loaded) return;

	if(!pImpl->frameTimer.NoFrame()) {
		// fps
		using namespace std::chrono;
		auto enterFrame = steady_clock::now();
		double frametime = duration<double>(enterFrame - pImpl->timer).count();
		pImpl->timer = enterFrame;

		auto time = duration_cast<milliseconds>(enterFrame.time_since_epoch());
		uint32_t timep = (time - duration_cast<hours>(time)).count();
		pImpl->fpsCounter.RegisterFrame(timep);

		auto& m_scene = (pImpl->scene);
		if(m_scene) {
			m_scene->shaderValueUpdater->MouseInput(m_mousePos[0]/pImpl->fboW, m_mousePos[1]/pImpl->fboH);
			m_scene->shaderValueUpdater->SetTexelSize(1.0f/pImpl->fboW, 1.0f/pImpl->fboH);
			pImpl->gm.Draw();
			// time elapsing
			auto nowFps = pImpl->fpsCounter.Fps();
			double idealtime = frametime;
			pImpl->timer = enterFrame;
			if(idealtime < 0.5) {
				m_scene->elapsingTime += idealtime;
				m_scene->frameTime = idealtime;
			}
		}
		pImpl->frameTimer.RenderFrame();
	}
}

void WallpaperGL::Clear()
{
	if(!m_inited) return;
	pImpl->gm.Destroy();
	LOG_INFO("Date cleared");
}

void WallpaperGL::SetAssets(const std::string& path) {
	std::string& assetsPath = fs::GetAssetsPath();
	assetsPath = path;
}

void WallpaperGL::SetUpdateCallback(const std::function<void()>& func) {
	pImpl->frameTimer.SetCallback(func);
	pImpl->frameTimer.Run();
}

void WallpaperGL::Start() {
	pImpl->frameTimer.Run();
}

void WallpaperGL::Stop() {
	pImpl->frameTimer.Stop();
}


void WallpaperGL::SetDefaultFbo(uint fbo, uint32_t w, uint32_t h) {
	pImpl->gm.SetDefaultFbo(fbo, w, h, m_fillMode);	
	pImpl->fboW = w;
	pImpl->fboH = h;
}

void WallpaperGL::SetFillMode(FillMode f) {
	m_fillMode = f;
	if(m_loaded) {
		pImpl->gm.ChangeFillMode(m_fillMode);
	}
}

uint32_t WallpaperGL::CurrentFps() const { return pImpl->fpsCounter.Fps(); }
uint8_t WallpaperGL::Fps() const { return pImpl->frameTimer.Fps(); }
void WallpaperGL::SetFps(uint8_t value) { return pImpl->frameTimer.SetFps(value); }

