#include "wallpaper.h"
#include <nlohmann/json.hpp>
#include <iostream>

typedef wallpaper::fs::file_node file_node;
using json = nlohmann::json;
using namespace wallpaper;

file_node wallpaper::WallpaperGL::m_pkgfs = file_node();
int WallpaperGL::m_objnum = -1;
int WallpaperGL::m_effnum = -1;

glm::mat4 GetAspectScaleMatrix(int width, int height, float aspect) {
	float fw = (float)width;
	float fh = (float)height;
	glm::mat4 result(1.0f);
	if(aspect*height > width) {
		result = glm::scale(result, glm::vec3(1.0f, (fw/aspect)/fh, 1.0f));	
	}else {
		result = glm::scale(result, glm::vec3((fh*aspect)/fw, 1.0f, 1.0f));	
	}
	return result;
}

bool WallpaperGL::Init(void *get_proc_address(const char *)) {
	LOG_INFO("Init opengl");
	m_inited = m_wpRender.Init(get_proc_address);
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
	m_wpRender.shaderMgr.globalUniforms.SetCamera(center,eye,up);

	auto& general_json = scene_json.at("general");
	if(!StringToVec<float>(general_json.at("clearcolor"), clearcolor)) return;
	m_wpRender.SetClearcolor(clearcolor);

	auto& ortho_j = general_json.at("orthogonalprojection");
	std::vector<int> ortho({ortho_j.at("width"), ortho_j.at("height")});
	m_wpRender.shaderMgr.globalUniforms.SetOrtho(ortho.at(0), ortho.at(1));
	m_wpRender.CreateGlobalFbo(ortho.at(0), ortho.at(1));
	
	if(m_flip)
		m_fboTrans = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f,0,0));
	gl::Shadervalue::SetShadervalues(m_shadervalues, "fboTrans", m_fboTrans);

	m_vertices = gl::VerticeArray::GenDefault(&m_wpRender.glWrapper);
	m_vertices.Update();

    auto& objects = scene_json.at("objects");
    for(auto& object:objects) {
		auto newobj = CreateObject(object);
		if(newobj)
			m_objects.emplace_back(std::move(newobj));
	}
	int index = 0;
    for(auto& iter:m_objects) {
		if(index++ == ObjNum()) break;
		if(!iter->Visible()) {
			LOG_INFO("\n-----Ignore invisiable object: " + iter->Name() + "------");
			continue;
		}
		LOG_INFO("\n-----Loading object: " + iter->Name() + "-----");
        iter->Load(m_wpRender);
	}
	// clean useless shadercache
	m_wpRender.shaderMgr.ClearShaderCache();
	// clear for first frame
	m_wpRender.shaderMgr.globalUniforms.ClearCache();
	m_loaded = true;
}

void WallpaperGL::Render(uint fbo, int width, int height) {
	if(!m_inited || !m_loaded) return;
	if(!m_wpRender.shaderMgr.globalUniforms.CacheEmpty()) {
		float lasttime = *(float*)m_wpRender.shaderMgr.globalUniforms.GetValue("g_Time");
		m_wpRender.shaderMgr.globalUniforms.ClearCache();
		float* nowtime = (float*)m_wpRender.shaderMgr.globalUniforms.GetValue("g_Time");
		int diff = (int)(*nowtime*1000) - (int)(lasttime*1000);
		if(diff < 1000 / 30 && diff > 0) {
			*nowtime = lasttime;
			return;
		}
		if(diff > 0)
			m_wpRender.timeDiffFrame = diff;
	}
	gl::GLFramebuffer defaultFbo(width, height);
	defaultFbo.framebuffer = fbo;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	CHECK_GL_ERROR_IF_DEBUG();
	m_wpRender.shaderMgr.globalUniforms.SetSize(width, height);
	m_wpRender.UseGlobalFbo();
	m_wpRender.Clear();

	int index = 0;
    for(auto& iter:m_objects){
		if(index++ == ObjNum()) break;
		if(!iter->Visible()) continue;
        iter->Render(m_wpRender);
	}

	glm::mat4 fboTrans(1.0f);
	if(m_keepAspect) {
		const auto& ortho = m_wpRender.shaderMgr.globalUniforms.Ortho();
		float aspect = (float)ortho.at(0) / (float)ortho.at(1);
		fboTrans = GetAspectScaleMatrix(defaultFbo.width, defaultFbo.height, aspect);
	}
	gl::Shadervalue::SetShadervalues(m_shadervalues, "fboTrans", fboTrans * m_fboTrans);


	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	m_wpRender.glWrapper.BindFramebufferViewport(&defaultFbo);
	CHECK_GL_ERROR_IF_DEBUG();
	m_wpRender.Clear();
	m_wpRender.glWrapper.ActiveTexture(0);
	m_wpRender.glWrapper.BindFramebufferTex(m_wpRender.GlobalFbo());
	m_wpRender.shaderMgr.BindShader("displayFbo");
	m_wpRender.shaderMgr.UpdateUniforms("displayFbo", m_shadervalues);
	m_vertices.Draw();
	// no gldelete
	defaultFbo.framebuffer = 0;
}

void WallpaperGL::Clear()
{
	if(!m_inited) return;
	m_wpRender.shaderMgr.globalUniforms.ClearCache();
	m_wpRender.shaderMgr.ClearCache();
	m_wpRender.texCache.Clear();	
    m_objects.clear();
	m_vertices.Delete();
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
