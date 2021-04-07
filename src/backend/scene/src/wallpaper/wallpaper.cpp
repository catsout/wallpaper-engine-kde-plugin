#include "wallpaper.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <ctime>
#include <thread>

typedef wallpaper::fs::file_node file_node;
using json = nlohmann::json;
using namespace wallpaper;

file_node wallpaper::WallpaperGL::m_pkgfs = file_node();
int WallpaperGL::m_objnum = -1;
int WallpaperGL::m_effnum = -1;

bool WallpaperGL::Init(void *get_proc_address(const char *)) {
	m_inited = m_gm.Initialize(get_proc_address);
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

    //read scene
    std::string scene_src = wallpaper::fs::GetContent(m_pkgfs, "scene.json");
	if(scene_src.empty()) {
		LOG_ERROR("Not supported scene type");
		return;
	}
	m_scene = m_parser.Parse(scene_src);	
	if(m_scene) {
		m_gm.InitializeScene(m_scene.get());
	}
	m_loaded = true;
}

void WallpaperGL::Render(uint fbo, int width, int height) {
	if(!m_inited || !m_loaded) return;

	if(!m_frameTimer.NoFrame()) {
		// fps
		using namespace std::chrono;
		auto enterFrame = steady_clock::now();
		auto time = duration_cast<milliseconds>(enterFrame.time_since_epoch());
		uint32_t timep = (time - duration_cast<hours>(time)).count();
		m_fpsCounter.RegisterFrame(timep);
		if(m_scene) {	
			m_scene->shaderValueUpdater->MouseInput(m_mousePos[0], m_mousePos[1]);
			m_gm.SetDefaultFbo(fbo, width, height);
			m_gm.Draw();
			// time elapsing
			double idealtime = 1.0f / (double)m_frameTimer.Fps();
			m_scene->elapsingTime += idealtime;
		}
		m_frameTimer.RenderFrame();
	}
}

void WallpaperGL::Clear()
{
	if(!m_inited) return;
	m_gm.Destroy();
	LOG_INFO("Date cleared");
}

void WallpaperGL::SetAssets(const std::string& path) {
	std::string& assetsPath = fs::GetAssetsPath();
	assetsPath = path;
}

void WallpaperGL::SetObjEffNum(int obj, int eff) {
	WallpaperGL::m_objnum = obj;
	WallpaperGL::m_effnum = eff;
}

void WallpaperGL::SetUpdateCallback(const std::function<void()>& func) {
	m_frameTimer.SetCallback(func);
	m_frameTimer.Run();
}

void WallpaperGL::Start() {
	m_frameTimer.Run();
}

void WallpaperGL::Stop() {
	m_frameTimer.Stop();
}
