#include "wallpaper.h"
#include <nlohmann/json.hpp>
#include <iostream>

typedef wallpaper::fs::file_node file_node;
using json = nlohmann::json;
using namespace wallpaper;

file_node wallpaper::WallpaperGL::m_pkgfs = file_node();
int WallpaperGL::m_objnum = -1;
int WallpaperGL::m_effnum = -1;

bool WallpaperGL::Init(void *get_proc_address(const char *)) {
//	if(m_inited) return true;
	LOG_INFO("Init opengl");
	m_inited = wpRender_.Init(get_proc_address);
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
    auto scene_json = json::parse(scene_src);

	auto& camera = scene_json.at("camera");
	std::vector<float> center,eye,up,clearcolor;
	if(!StringToVec<float>(camera.at("center"), center)) return;
	if(!StringToVec<float>(camera.at("eye"), eye)) return;
	if(!StringToVec<float>(camera.at("up"), up)) return;
	wpRender_.shaderMgr.globalUniforms.SetCamera(center,eye,up);

	auto& general_json = scene_json.at("general");
	if(!StringToVec<float>(general_json.at("clearcolor"), clearcolor)) return;
	wpRender_.SetClearcolor(clearcolor);

	auto& ortho = general_json.at("orthogonalprojection");
	wpRender_.shaderMgr.globalUniforms.SetOrtho(ortho.at("width"), ortho.at("height"));
		
	wpRender_.CreateGlobalFbo(ortho.at("width"), ortho.at("height"));

	auto size = wpRender_.shaderMgr.globalUniforms.Ortho();
	vertices_ = gl::VerticeArray::GenDefault(&wpRender_.glWrapper);
	vertices_.Update();

    auto& objects = scene_json.at("objects");
    for(auto& object:objects) {
		auto newobj = CreateObject(object);
		if(newobj)
			m_objects.emplace_back(std::move(newobj));
	}
	int index = 0;
    for(auto& iter:m_objects) {
		LOG_INFO("\n-----Loading object: " + iter->Name() + "-----");
        iter->Load(wpRender_);
	}
	// clear for first frame
	wpRender_.shaderMgr.globalUniforms.ClearCache();
	m_loaded = true;
}

void WallpaperGL::Render(uint fbo, int width, int height) {
	if(!m_inited || !m_loaded) return;
	if(!wpRender_.shaderMgr.globalUniforms.CacheEmpty()) {
		float lasttime = *(float*)wpRender_.shaderMgr.globalUniforms.GetValue("g_Time");
		wpRender_.shaderMgr.globalUniforms.ClearCache();
		float* nowtime = (float*)wpRender_.shaderMgr.globalUniforms.GetValue("g_Time");
		int diff = (int)(*nowtime*1000) - (int)(lasttime*1000);
		if(diff < 1000 / 30 && diff > 0) {
			*nowtime = lasttime;
			return;
		}
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	CHECK_GL_ERROR_IF_DEBUG();
	wpRender_.shaderMgr.globalUniforms.SetSize(width, height);
	wpRender_.UseGlobalFbo();
	wpRender_.Clear();

	int index = 0;
    for(auto& iter:m_objects){
		if(index++ == ObjNum()) break;
        iter->Render(wpRender_);
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	CHECK_GL_ERROR_IF_DEBUG();
	wpRender_.glWrapper.Viewport(0, 0, width, height);
	wpRender_.Clear();
	wpRender_.glWrapper.ActiveTexture(0);
	wpRender_.glWrapper.BindTexture(&(wpRender_.GlobalFbo()->color_texture));
	wpRender_.shaderMgr.BindShader("displayFbo");
	wpRender_.shaderMgr.UpdateUniforms("displayFbo", {});
	vertices_.Draw();
	CHECK_GL_ERROR_IF_DEBUG();
}

void WallpaperGL::Clear()
{
	if(!m_inited) return;
	wpRender_.shaderMgr.globalUniforms.ClearCache();
	wpRender_.shaderMgr.ClearCache();
	wpRender_.texCache.Clear();	
    m_objects.clear();
	if(m_loaded) vertices_.Delete();
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
