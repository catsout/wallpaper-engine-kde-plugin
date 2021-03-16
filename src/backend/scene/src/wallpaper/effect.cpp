#include "effect.h"
#include "pkg.h"
#include "wallpaper.h"

using json = nlohmann::json;
using namespace wallpaper;

bool Effect::From_json(const nlohmann::json& effect_j) {
	auto& pkgfs = WallpaperGL::GetPkgfs();
	// effect code is at "file"
	std::string file_src = fs::GetContent(pkgfs, effect_j.at("file"));
	if(file_src.empty()) {
		m_name = "unknown";
		return false;
	}
	auto file_j = json::parse(file_src); 

	GET_JSON_NAME_VALUE(file_j, "name", m_name);

	GET_JSON_NAME_VALUE_NOWARN(effect_j, "visible", m_visible);

	// load fbos
	if(file_j.contains("fbos")) {
		auto& fbos = file_j.at("fbos");
		for(auto& f:fbos) {
			std::string name;
			float scale = 1.0f;
			GET_JSON_NAME_VALUE(f, "name", name);
			GET_JSON_NAME_VALUE(f, "scale", scale);
			m_fboDataMap[name] = FboData(scale);
		}
	}

	// effect code must have passes, which contain material
	if(!file_j.contains("passes")) return false;	
	auto& file_passes_j = file_j.at("passes");
	// pass effect passes to effect code
	if(effect_j.contains("passes")) {
		auto& passes_j = effect_j.at("passes");
		auto it_file=file_passes_j.begin();
		for(auto& el:passes_j)
			(*it_file++)["material_pass"] = el;
	}
	int index = 0;
	for(auto& pass_j:file_passes_j) {
		//load material
		if(!pass_j.contains("material")) return false;
		std::string material_src = fs::GetContent(pkgfs, pass_j.at("material"));
		auto material_j = json::parse(material_src);
		auto& content_j = material_j.at("passes")[0];
		// pass effect passes to material
		if(pass_j.contains("material_pass")) {
			auto& material_pass_j = pass_j.at("material_pass");
			for(auto& el:material_pass_j.items())
				content_j[el.key()] = el.value();
		}
		m_materials.emplace_back((RenderObject&)m_imgObject, m_size);
		auto& material = m_materials.back();
		material.material.From_json(material_j);
		//load bind
		if(pass_j.contains("bind")) {
			auto& binds_j = pass_j.at("bind");
			for(auto& b:binds_j) {
				BindInfo bi;
				GET_JSON_NAME_VALUE(b, "name", bi.name);
				GET_JSON_NAME_VALUE(b, "index", bi.index);
				material.bindInfos.push_back(bi);
			}
		}
		else material.bindInfos.push_back({"previous",0});
		material.target = pass_j.contains("target")?pass_j.at("target"):"default";
		index++;
	}
	return true;
}

void Effect::Load(WPRender& wpRender) {
	m_vertices = gl::VerticeArray::GenSizedBox(&wpRender.glWrapper, std::vector<float>(m_size.begin(),m_size.end()));
	m_vertices.Update();
    m_vertices_default = gl::VerticeArray::GenDefault(&wpRender.glWrapper);
	m_vertices_default.Update();

	for(auto& f:m_fboDataMap) {
		float scale = f.second.scale;
		f.second.fbo = std::unique_ptr<gl::GLFramebuffer>(wpRender.glWrapper.CreateFramebuffer(m_size[0]/scale, m_size[1]/scale));
	}

	auto model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(m_size[0]/2.0f, m_size[1]/2.0f, 0.0f));
	auto viewpro_mat = glm::ortho(0.0f, (float)m_size[0], 0.0f, (float)m_size[1], -100.0f, 100.0f);
	auto size_modelviewpro_mat = viewpro_mat * model_mat;
	for(auto& m:m_materials) {
		m.material.Load(wpRender);
		// after material Load
		glm::mat4 modelviewpro_mat(1.0f);
		if(wpRender.shaderMgr.ShaderContainUnifom(m.material.GetShader(), "g_ModelViewProjectionMatrix")) {
			modelviewpro_mat = size_modelviewpro_mat;
			m.material.SetVertices(&m_vertices);
		}
		else {
			modelviewpro_mat = glm::mat4(1.0f);
			m.material.SetVertices(&m_vertices_default);
		}

		gl::Shadervalue::SetShadervalues(m.material.GetShadervalues(), "g_ModelViewProjectionMatrix", modelviewpro_mat);
		gl::Shadervalue::SetShadervalues(m.material.GetShadervalues(), "g_ModelViewProjectionMatrixInverse", glm::inverse(modelviewpro_mat));
	}
}
		
void Effect::Render(WPRender& wpRender) {
	for(auto& m:m_materials) {
		bool switchFbo = false;
		if(m.target == "default") {
			wpRender.glWrapper.BindFramebufferViewport(m_imgObject.TargetFbo());
			switchFbo = true;
		}
		else {
			FboData& targetFboData = m_fboDataMap.at(m.target);
			wpRender.glWrapper.BindFramebufferViewport(targetFboData.fbo.get());
		}
		wpRender.Clear(0.0f);

		for(auto& b:m.bindInfos) {
			wpRender.glWrapper.ActiveTexture(b.index);
			if(b.name == "previous")
				wpRender.glWrapper.BindTexture(&(m_imgObject.CurFbo())->color_texture);
			else
				wpRender.glWrapper.BindTexture(&m_fboDataMap.at(b.name).fbo->color_texture);
		}
	    m.material.Render(wpRender);
		if(switchFbo)	
			m_imgObject.SwitchFbo();
	}
}
