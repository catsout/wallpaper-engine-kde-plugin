#include "wallpaper.h"
#include "Utils/Logging.h"

#include "Utils/FpsCounter.h"
#include "Utils/FrameTimer.h"

#include "Fs/VFS.h"
#include "Fs/PhysicalFs.h"
#include "WPPkgFs.h"

#include "Scene/SceneMesh.h"
#include "Scene/Scene.h"
#include "WPSceneParser.h"
#include "Renderer/GLGraphicManager.h"

#include <chrono>
#include <thread>

using namespace wallpaper;

class WallpaperGL::impl {
public:
	impl() = default;
	~impl()	{ frameTimer.Stop(); }
	FpsCounter fpsCounter;
	FrameTimer frameTimer;
	WPSceneParser parser;
	GLGraphicManager gm;

	std::chrono::time_point<std::chrono::steady_clock> timer {std::chrono::steady_clock::now()};
	std::unique_ptr<Scene> scene;
	float fboW {1920.0f};
	float fboH {1080.0f};

	fs::VFS vfs;
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
	if(!pImpl->vfs.IsMounted("assets")) {
		bool sus = pImpl->vfs.Mount("/assets",
			fs::CreatePhysicalFs(m_assetsPath),
			"assets"
		);
		if(!sus) return;
	}
	if(!pImpl->vfs.Mount("/assets", fs::WPPkgFs::CreatePkgFs(pkg_path))) {
		LOG_ERROR("Can't load pkg file: %s", pkg_path.c_str());
		return;
	}

	m_pkgPath = pkg_path;
	std::string scene_src;
	if(pImpl->vfs.Contains("/assets/scene.json")) {
		auto f = pImpl->vfs.Open("/assets/scene.json");
		if(f) scene_src = f->ReadAllStr();
	}
	else if(pImpl->vfs.Contains("/assets/gifscene.json")) {
		auto f = pImpl->vfs.Open("/assets/gifscene.json");
		if(f) scene_src = f->ReadAllStr();
	}
	if(scene_src.empty()) {
		LOG_ERROR("Not supported scene type");
		return;
	}
	pImpl->scene = pImpl->parser.Parse(scene_src, pImpl->vfs);	
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
	pImpl->vfs.Unmount("/assets");
	LOG_INFO("Date cleared");
}

void WallpaperGL::SetAssets(const std::string& path) {
	m_assetsPath = path;
}

void WallpaperGL::SetUpdateCallback(const std::function<void()>& func) {
	std::function<void()> copyf = func;
	// the callback runs on other thread, but it will be stopped before pImpl destructed
	// so use raw point
	auto* pImplRaw = pImpl.get();
	auto wrapperf = [copyf,pImplRaw]() {
		copyf();
		if(pImplRaw->scene)
			pImplRaw->scene->paritileSys.Emitt();
	};
	pImpl->frameTimer.SetCallback(wrapperf);
	pImpl->frameTimer.Run();
}

void WallpaperGL::Start() {
	pImpl->frameTimer.Run();
}

void WallpaperGL::Stop() {
	pImpl->frameTimer.Stop();
}


void WallpaperGL::SetDefaultFbo(uint fbo, uint16_t w, uint16_t h) {
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

void WallpaperGL::SetFlip(bool value) {
	pImpl->gm.SetFlip(false, value);
}

void WallpaperGL::OutGraphviz(const std::string& path) {
	pImpl->gm.OutGriphviz(path);
}

