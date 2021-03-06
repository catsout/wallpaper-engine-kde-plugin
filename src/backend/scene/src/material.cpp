#include <iostream>
#include "material.h"
#include "pkg.h"
#include "wallpaper.h"
#include "teximage.h"

using json = nlohmann::json;
namespace wp = wallpaper;

void wp::Blending::ApplayBlending(wp::Blending::Type type) {
	switch(type) {
	case Blending::normal:
		glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
		break;
	case Blending::translucent:
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
		break;
	case Blending::additive:
		glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
		break;
	}
}

wp::Material::Material(RenderObject& object, const std::vector<int>& size):m_object(object),m_size(size) {
};

bool wp::Material::From_json(const json& obj_json) {
	const ImageObject& imgobj = dynamic_cast<const ImageObject&>(m_object);
	m_combos = gl::Combos(imgobj.BaseCombos());

    if(!obj_json.contains("passes")) return false;
    auto& content = obj_json.at("passes")[0];

	if(!GET_JSON_NAME_VALUE(content, "shader", m_shader)) return false;

	if(content.contains("textures"))
		for(auto& t:content.at("textures"))
			textures_.push_back(t.is_null()?"":t);

	std::string blending("translucent");
	if(GET_JSON_NAME_VALUE(content, "blending", blending)) {
		if(blending == "translucent")
			m_blending = Blending::translucent;
		else if(blending == "additive")
			m_blending = Blending::additive;
		else if(blending == "normal")
			m_blending = Blending::normal;
		else LOG_ERROR("Unknown blending type" + blending);
	}

	std::string depthtest("disabled");
	if(GET_JSON_NAME_VALUE(content, "depthtest", depthtest))
        m_depthtest = depthtest == "disabled" ? false : true;
	
	std::string cullmode("nocull");
	if(GET_JSON_NAME_VALUE(content, "depthtest", depthtest)) {
		if(cullmode != "nocull") 
			LOG_INFO("Not supported cullmode yet, " + cullmode);
	}

	if(content.contains("combos")) {
		for(auto& c:content.at("combos").items()) {
			std::string name;
			int value;
			GET_JSON_VALUE(c.key(), name);
			GET_JSON_VALUE(c.value(), value);
			std::transform(name.begin(), name.end(), name.begin(), ::toupper);
			m_combos[name] = value;
		}
	}

	if(content.contains("constantshadervalues")) {
		m_constShadervalues = content.at("constantshadervalues").dump();
	}
    return true;
}

void wp::Material::Load(WPRender& wpRender) {
	m_shader = wpRender.shaderMgr.CreateShader(m_shader, m_combos, m_shadervalues, textures_.size());
	auto* lks = wpRender.shaderMgr.CreateLinkedShader(m_shader);
	/*
	for(auto& el:lks->GetUniforms()) {
		LOG_INFO(el.name);
	}*/
	wpRender.shaderMgr.SetTextures(m_shader, m_shadervalues);
	// load constShadervalues as glname 
	if(!m_constShadervalues.empty()) {
		auto m_constShadervaluesjson = json::parse(m_constShadervalues);
		for(auto& c:m_constShadervaluesjson.items()) {
			std::string glname = gl::Shadervalue::FindShadervalue(m_shadervalues, c.key());
			if(!glname.empty()) {
				auto& sv = m_shadervalues[glname];
				std::vector<float> value;	
				if(c.value().is_string() || (c.value().contains("value") && c.value().at("value").is_string()))
					GET_JSON_VALUE(c.value(), sv.value);
				else {
					sv.value.resize(1);
					GET_JSON_VALUE(c.value(), sv.value.at(0));
				}
			}
		}
	}

	std::vector<gl::Shadervalue> sv_resolutions;
	for(auto&el:m_shadervalues) {
		if(el.second.glname.compare(0, 9, "g_Texture") == 0 && el.second.glname.size()<11) {
			int index = std::stoi(&el.second.glname[9]);
			
			if(index >= textures_.size()) 
				textures_.resize(index+1, "_rt_replace");
			if(!el.second.value_str.empty() && textures_[index] == "_rt_replace") {
					textures_[index] = el.second.value_str;
			}
			auto& texture = textures_[index];

			//resolution and load tex
			std::string sv_resolution = "g_Texture"+std::to_string(index)+"Resolution";
			sv_resolutions.push_back(gl::Shadervalue());
			sv_resolutions.back().glname = sv_resolution;

			if(texture.empty()) {
				std::vector<int> re = {m_size[0],m_size[1],m_size[0],m_size[1]};
				sv_resolutions.back().value = std::vector<float>(re.begin(),re.end());
				continue;
			}else if(texture.compare(0, 4, "_rt_") == 0) {
				std::vector<int> re_i = {m_size[0],m_size[1],m_size[0],m_size[1]};
				std::vector<float> re(re_i.begin(), re_i.end());
				if(texture.compare(4, 14, "FullFrameBuffer") == 0) {
					const auto& ortho = wpRender.shaderMgr.globalUniforms.Ortho();
					std::vector<float> ortho_f(ortho.begin(), ortho.end());
					re = std::vector<float>({ortho_f[0], ortho_f[1], ortho_f[0], ortho_f[1]});
				}
				sv_resolutions.back().value = re;
				continue;
			}
			gl::Texture* tex = wpRender.texCache.LoadTexture(texture);
			auto resolution = tex->GetResolution();
			sv_resolutions.back().value = std::vector<float>(resolution.begin(), resolution.end());
		}
	}
	for(auto& el:sv_resolutions)
		m_shadervalues[el.glname] = el;
}

void wp::Material::Render(WPRender& wpRender) {
	wpRender.shaderMgr.BindShader(m_shader);
	for(int i=0;i < textures_.size();i++){
		if(textures_[i].empty() || textures_[i].compare(0,4,"_rt_") == 0) 
			continue;
		wpRender.glWrapper.ActiveTexture(i);
		auto* tex = wpRender.texCache.GetTexture(textures_[i]);
		if(tex->IsSprite()) {
			auto sf = tex->NextSpriteFrame(wpRender.timeDiffFrame);
			if(sf != nullptr) {
				using namespace gl;
				std::string texgl = "g_Texture" +std::to_string(i);
				Shadervalue::SetShadervalues(m_shadervalues, texgl + "Translation", SpriteFrame::GetTranslation(*sf));
				Shadervalue::SetShadervalues(m_shadervalues, texgl + "Rotation", SpriteFrame::GetRotation(*sf));
				tex->SwitchTex(sf->imageId);
			}
		}
		tex->Bind();
	}
	wpRender.shaderMgr.BindShader(m_shader);
	wpRender.shaderMgr.UpdateUniforms(m_shader, m_shadervalues);
	if(m_depthtest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	//Blending::ApplayBlending(m_blending);		
	CHECK_GL_ERROR_IF_DEBUG();
	m_vertices->Draw();
	CHECK_GL_ERROR_IF_DEBUG();
}
