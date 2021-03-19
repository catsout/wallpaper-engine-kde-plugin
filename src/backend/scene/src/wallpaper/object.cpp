#include "object.h"
#include "common.h"
#include "pkg.h"
#include "wallpaper.h"
#include <iostream>
#include <string>
/*
    Visible: controls whether a renderable is drawn to the screen. Useful to prerender 2D renderables offscreen and use them as inputs to other renderables.
    Name: defines a name to identify the renderable more easily in the scene graph.
    Origin: is the position in XYZ coordinates.
    Angles: describes the rotation in degrees and euler angles.
    Scale: is the size along the XYZ axis.
    Blend mode: works like photoshop blend modes for 2D renderables.
    Parallax depth: for 2D renderables if the camera has parallax enabled in the scene options. This controls how much a renderable is affected by the effect, making it possible to create an illusion of depth.
    Materials: a list of materials that are used by the renderable.
*/

using json = nlohmann::json;
using namespace wallpaper;

std::unique_ptr<RenderObject> wallpaper::CreateObject(const json& obj_json) {
	std::string name;
	GET_JSON_NAME_VALUE(obj_json, "name", name);

    if(obj_json.contains("image") && !obj_json.at("image").is_null()) {
        auto obj = std::make_unique<ImageObject>();
        if(obj->From_json(obj_json)) return obj;
		else LOG_ERROR("Read image object " + name + " from json failed");
    }
    else if(obj_json.contains("particle") && !obj_json.at("particle").is_null()) {
        auto obj = std::make_unique<ParticleObject>();
        if(obj->From_json(obj_json)) return obj;
    }
    return nullptr;
}

bool wallpaper::RenderObject::From_json(const json& obj)
{
	if(!GET_JSON_NAME_VALUE(obj, "name", m_name))
		return false;

	GET_JSON_NAME_VALUE(obj, "origin", m_origin);

	GET_JSON_NAME_VALUE(obj, "scale", m_scale);

	GET_JSON_NAME_VALUE(obj, "angles", m_angles);

	GET_JSON_NAME_VALUE_NOWARN(obj, "parallaxDepth", m_parallaxDepth);

	GET_JSON_NAME_VALUE_NOWARN(obj, "visible", m_visible);

    return true;
}

ImageObject::~ImageObject() {
}

bool ImageObject::From_json(const json& obj)
{
    if(!RenderObject::From_json(obj) || !obj.contains("image")) return false;
	if(Name() == "Compose")
		m_compose = true;
	if(obj.contains("size")) {
		std::vector<float> fsize;
		if(!StringToVec<float>(obj.at("size"), fsize)) return false;
		m_size = std::vector<int>(fsize.begin(), fsize.end());
	}
	GET_JSON_NAME_VALUE_NOWARN(obj, "copybackground", m_copybackground);

	GET_JSON_NAME_VALUE_NOWARN(obj, "brightness", m_brightness);

	GET_JSON_NAME_VALUE_NOWARN(obj, "colorBlendMode", m_blendmode);

	GET_JSON_NAME_VALUE_NOWARN(obj, "alpha", m_alpha);

	GET_JSON_NAME_VALUE_NOWARN(obj, "color", m_color);

    std::string image_str = fs::GetContent(WallpaperGL::GetPkgfs(), obj.at("image"));
    /*
    {
        "autosize" : true,
        "material" : "*.json"
    }
    */
    auto image = json::parse(image_str);

    if(!image.contains("material")) return false;
	if(!obj.contains("size")) {
		if(image.contains("width")) {
			m_size[0] = image.at("width");
			m_size[1] = image.at("height");
		}
		else if(image.contains("fullscreen")) {
			m_fullscreen = true;
		}
		else return false;
	}
		
	GET_JSON_NAME_VALUE_NOWARN(obj, "autosize", m_autosize);

	GenBaseCombos();

    std::string material_str = fs::GetContent(WallpaperGL::GetPkgfs(), image.at("material"));
	auto mat_j = json::parse(material_str);
    if(!m_material.From_json(mat_j)) return false;
	

	m_material.SetSize(m_size);
	if(obj.contains("effects")) {
		for(auto& e:obj.at("effects")) {
			m_effects.push_back(Effect(*this, m_size));
			if(!m_effects.back().From_json(e)) {
				LOG_INFO("object: " + Name() + "'s effect: " + m_effects.back().Name() + " not load");
				m_effects.pop_back();
			}
		}
	}
	if(m_blendmode != 0) {
		material_str = fs::GetContent(WallpaperGL::GetPkgfs(), "materials/util/effectpassthrough.json");
		auto matEff_j = json::parse(material_str);
		auto& combos = matEff_j.at("passes")[0].at("combos");
		combos["BONECOUNT"] = 1;
		combos["BLENDMODE"] = m_blendmode;
		if(!m_materialEffePass.From_json(matEff_j)) return false;
		m_materialEffePass.SetSize(m_size);
	}
    return true;
}

void ImageObject::Load(WPRender& wpRender)
{
	// fullscreen
	if(m_fullscreen) {
		m_size = wpRender.shaderMgr.globalUniforms.Ortho();
		Origin() = std::vector<float>({m_size[0]/2.0f, m_size[1]/2.0f, 0.0f});
	}
	auto ori = Origin();
	ori[1] = wpRender.shaderMgr.globalUniforms.Ortho()[1] - ori[1];
	auto& verArry = Vertices();
	verArry = gl::VerticeArray::GenSizedBox(&wpRender.glWrapper, std::vector<float>(m_size.begin(),m_size.end()));
    verArry.Update();

    m_material.Load(wpRender);
	if(m_blendmode != 0) {
		m_materialEffePass.Load(wpRender);
		m_materialEffePass.GetTextures()[1] = "_rt_";
	}
	
	// fbo shadervalues	
	auto viewpro_mat = wpRender.shaderMgr.globalUniforms.GetViewProjectionMatrix();
	auto scale = Scale();
	//3. move to origin
	auto model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(ori[0],ori[1],ori[2]));
	//2. rotation
	model_mat = glm::rotate(model_mat, -Angles()[2], glm::vec3(0,0,1)); // z, need negative
	model_mat = glm::rotate(model_mat, Angles()[0], glm::vec3(1,0,0)); // x
	model_mat = glm::rotate(model_mat, Angles()[1], glm::vec3(0,1,0)); // y
	//1. scale
	model_mat = glm::scale(model_mat, glm::vec3(scale[0], scale[1], scale[2]));
	m_fboTrans = viewpro_mat * model_mat;
	gl::Shadervalue::SetShadervalues(shadervalues_, "g_ModelViewProjectionMatrix", m_fboTrans);

	// material shadervalues
	model_mat = glm::mat4(1.0f);
	if(m_material.GetShadervalues().count("g_Texture0Resolution") != 0) {
		// remove black edge
		auto& sv = m_material.GetShadervalues().at("g_Texture0Resolution");
		if(sv.value[0] != sv.value[2] || sv.value[1] != sv.value[3]) {
			model_mat = glm::translate(glm::mat4(1.0f), -glm::vec3(m_size[0]/2.0f,m_size[1]/2.0f,0.0f));
			model_mat = glm::scale(model_mat, glm::vec3(sv.value[0]/sv.value[2], sv.value[1]/sv.value[3], 1.0f));
			model_mat = glm::translate(model_mat, glm::vec3(m_size[0]/2.0f,m_size[1]/2.0f,0.0f));
		}
	}
	if(IsCompose()) {
		// compose need to know wordcoord and use global ViewProjectionMatrix
		// this is ok for fullscreen as ori at (0,0,0)
		model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(ori[0], ori[1], ori[2])) * model_mat;
		model_mat = glm::scale(model_mat, glm::vec3(scale[0], scale[1], scale[2]));
	} else {
		model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(m_size[0]/2.0f, m_size[1]/2.0f, 0.0f)) * model_mat;
		viewpro_mat = glm::ortho(0.0f, (float)m_size[0], 0.0f, (float)m_size[1], -100.0f, 100.0f);
	}
	auto modelviewpro_mat = viewpro_mat * model_mat;
	gl::Shadervalue::SetShadervalues(m_material.GetShadervalues(), "g_ModelViewProjectionMatrix", modelviewpro_mat);
	gl::Shadervalue::SetShadervalues(m_material.GetShadervalues(), "g_UserAlpha", m_alpha);
	gl::Shadervalue::SetShadervalues(m_material.GetShadervalues(), "g_Brightness", m_brightness);
	gl::Shadervalue::SetShadervalues(m_material.GetShadervalues(), "g_Alpha", m_alpha);
	gl::Shadervalue::SetShadervalues(m_material.GetShadervalues(), "g_Color", m_color);


	int index = 0;
	for(auto& e:m_effects){
		if(index++ == WallpaperGL::EffNum()) break;
		LOG_INFO("\n---Loading effect---");
		e.Load(wpRender);
	}
	m_fbo1 = std::unique_ptr<gl::GLFramebuffer>(wpRender.glWrapper.CreateFramebuffer(m_size[0], m_size[1]));
	if(m_effects.size() > 0)
		m_fbo2 = std::unique_ptr<gl::GLFramebuffer>(wpRender.glWrapper.CreateFramebuffer(m_size[0], m_size[1]));
}

void ImageObject::Render(WPRender& wpRender)
{
	m_curFbo = m_fbo1.get();

	wpRender.glWrapper.BindFramebufferViewport(m_curFbo);

	if(m_copybackground || m_fullscreen)	{
		wpRender.Clear(0.0f);
		if(IsCompose() || m_fullscreen) {
			wpRender.glWrapper.ActiveTexture(0);
			wpRender.glWrapper.BindFramebufferTex(wpRender.GlobalFbo());
		}
	}
	else {
		wpRender.Clear();
	}		

	glDisable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ZERO);
	m_material.SetVertices(&Vertices());
    m_material.Render(wpRender);


	int index = 0;
	for(auto& e:m_effects) {
		if(index++ == WallpaperGL::EffNum()) break;
		e.Render(wpRender);
	}

	// parallaxDepth
	if(wpRender.GetCameraParallax().enable) {
		const auto& camParaVec = wpRender.GetCameraParallaxVec();
		const auto& depth = ParallaxDepth();
		auto transVec = glm::vec3(camParaVec[0]*depth[0], camParaVec[1]*depth[1], 0.0f);
		auto trans = glm::translate(glm::mat4(1.0f), transVec);
		gl::Shadervalue::SetShadervalues(shadervalues_, "g_ModelViewProjectionMatrix", trans*m_fboTrans);
	}
	if(m_blendmode != 0) {
		glDisable(GL_BLEND);
		wpRender.glWrapper.BindFramebufferViewport(wpRender.GlobalFbo());
		m_materialEffePass.GetShadervalues() = shadervalues_;

		auto* tex = wpRender.glWrapper.CopyTexture(wpRender.GlobalFbo());
		wpRender.glWrapper.BindFramebufferViewport(wpRender.GlobalFbo());
		wpRender.glWrapper.ActiveTexture(0);
		wpRender.glWrapper.BindFramebufferTex(CurFbo());

		wpRender.glWrapper.ActiveTexture(1);
		wpRender.glWrapper.BindTexture(tex);

		m_materialEffePass.SetVertices(&Vertices());
		m_materialEffePass.Render(wpRender);	
		wpRender.glWrapper.DeleteTexture(tex);
		delete tex;
	} else {
		glEnable(GL_BLEND);
		Blending::ApplayBlending(m_material.Blending());
	//	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
//		wpRender.UseGlobalFbo(shadervalues_);
		wpRender.glWrapper.BindFramebufferViewport(wpRender.GlobalFbo());
		wpRender.shaderMgr.BindShader("passthrough+TRANSFORM1");
		wpRender.shaderMgr.UpdateUniforms("passthrough+TRANSFORM1", shadervalues_);
		wpRender.glWrapper.ActiveTexture(0);
		wpRender.glWrapper.BindFramebufferTex(CurFbo());
		Vertices().Draw();
	}
}


bool ImageObject::IsCompose() const {
	return m_compose;
}

void ImageObject::GenBaseCombos() {
	m_basecombos.clear();
	//{"material":"ui_editor_properties_composite","combo":"COMPOSITE","type":"options","default":0,"options":{"ui_editor_properties_normal":0,"ui_editor_properties_blend":1,"ui_editor_properties_under":2,"ui_editor_properties_cutout":3}}
	// used for blur_combine 
	m_basecombos["COMPOSITE"] = 1;//IsCompose()?0:1;
	if(m_fullscreen)
		m_basecombos["TRANSFORM"] = 1;
}


gl::GLFramebuffer* ImageObject::CurFbo() {
	if(m_curFbo == nullptr)
		m_curFbo = m_fbo1.get();
	return m_curFbo;
}

gl::GLFramebuffer* ImageObject::TargetFbo() {
	if(m_fbo1.get() == m_curFbo)
		return m_fbo2.get();
	else return m_fbo1.get();
}

void ImageObject::SwitchFbo() {
	if(m_fbo1.get() == m_curFbo)	
		m_curFbo = m_fbo2.get();
	else m_curFbo = m_fbo1.get();
}


ParticleObject::ParticleObject() {
}

bool ParticleObject::From_json(const json& obj) {
    if(!RenderObject::From_json(obj)) return false;
    return true;
}

void ParticleObject::Load(WPRender& wpRender) {
    LOG_INFO("Particle Object(not supported): " + Name());
}
