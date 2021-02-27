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

std::unique_ptr<RenderObject> wallpaper::CreateObject(const json& obj_json)
{
    if(obj_json.contains("image") && !obj_json.at("image").is_null()) {
        auto obj = std::make_unique<ImageObject>();
        if(obj->From_json(obj_json)) return obj;
    }
    else if(obj_json.contains("particle") && !obj_json.at("particle").is_null()) {
        auto obj = std::make_unique<ParticleObject>();
        if(obj->From_json(obj_json)) return obj;
    }
    return nullptr;
}

bool wallpaper::RenderObject::From_json(const json& obj)
{

    m_name = obj.at("name");
	//if(!(obj.contains("origin") && obj.contains("angles") && obj.contains("scale")));
	//	return false;
	if(!(obj.contains("origin") && obj.at("origin").is_string())) return false;
    if(!StringToVec<float>(obj.at("origin"), m_origin)) return false;

	if(!(obj.contains("scale") && obj.at("scale").is_string())) return false;
    if(!StringToVec<float>(obj.at("scale"), m_scale)) return false;

	if(!(obj.contains("angles") && obj.at("angles").is_string())) return false;
    if(!StringToVec<float>(obj.at("angles"), m_angles)) return false;

    if(obj.contains("visible") && obj.at("visible").is_boolean()) m_visible = obj.at("visible");
    return true;
}

ImageObject::~ImageObject() {
}

bool ImageObject::From_json(const json& obj)
{
    if(!RenderObject::From_json(obj) || !obj.contains("image")) return false;

	if(obj.contains("size")) {
		std::vector<float> fsize;
		if(!StringToVec<float>(obj.at("size"), fsize)) return false;
		size_ = std::vector<int>(fsize.begin(), fsize.end());
	}
	if(obj.contains("copybackground"))
		copybackground_ = obj.at("copybackground");

	if(obj.contains("alpha") && obj.at("color").is_number())
		alpha_ = obj.at("alpha");

	if(obj.contains("color") && obj.at("color").is_string())
		if(!StringToVec<float>(obj.at("color"), color_)) return false;

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
			size_[0] = image.at("width");
			size_[1] = image.at("height");
		}
		else return false;
	}
		
	if(image.contains("autosize"))
		autosize_ = image.at("autosize").get<bool>();
    std::string material_str = fs::GetContent(WallpaperGL::GetPkgfs(), image.at("material"));
    if(!m_material.From_json(json::parse(material_str))) return false;
	m_material.SetSize(size_);
	if(obj.contains("effects")) {
		for(auto& e:obj.at("effects")) {
			effects_.push_back(Effect(*this, size_));
			if(!effects_.back().From_json(e))
				effects_.pop_back();
		}
	}
    return true;
}

void SetShadervalue(gl::Shadervalues& shadervalues, const std::string& glname, const std::vector<float>& value) {
	gl::Shadervalue sv = gl::Shadervalue();
	sv.glname = glname;
	sv.value = value;
	shadervalues[glname] = sv;
}

void ImageObject::Load(WPRender& wpRender)
{
	auto ori = Origin();
	ori[1] = wpRender.shaderMgr.globalUniforms.Ortho()[1] - ori[1];
	auto& verArry = Vertices();
	verArry = gl::VerticeArray::GenSizedBox(&wpRender.glWrapper, std::vector<float>(size_.begin(),size_.end()));
    verArry.Update();

    m_material.Load(wpRender);
	
	auto viewpro_mat = wpRender.shaderMgr.globalUniforms.GetViewProjectionMatrix();
	auto scale = Scale();
	//2. move to origin
	auto model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(ori[0],ori[1],ori[2]) - glm::vec3(size_[0]*scale[0]/2.0f, size_[1]*scale[1]/2.0f, ori[2]));
	//1. scale
	model_mat = glm::scale(model_mat, glm::vec3(scale[0], scale[1], scale[2]));

	auto modelviewpro_mat = viewpro_mat * model_mat;
	const float* modelviewpro  = glm::value_ptr(modelviewpro_mat);
	SetShadervalue(shadervalues_,"g_ModelViewProjectionMatrix", std::vector<float>(modelviewpro, modelviewpro + 4*4));

	if(m_material.GetShadervalues().count("g_Texture0Resolution") != 0) {
		auto& sv = m_material.GetShadervalues().at("g_Texture0Resolution");
		model_mat = glm::scale(glm::mat4(1.0f), glm::vec3(sv.value[0]/sv.value[2], sv.value[1]/sv.value[3], 1.0f));
		if(Name() == "Compose") {
			// compose need to know wordcoord and use global ViewProjectionMatrix
			model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(ori[0] - size_[0]/2.0f, ori[1] - size_[1]/2.0f, ori[2])) * model_mat;
		} else {
			viewpro_mat = glm::ortho(0.0f, (float)size_[0], 0.0f, (float)size_[1], -100.0f, 100.0f);
		}
		modelviewpro_mat = viewpro_mat * model_mat;
		modelviewpro  = glm::value_ptr(modelviewpro_mat);
		m_material.SetShadervalue("g_ModelViewProjectionMatrix", std::vector<float>(modelviewpro, modelviewpro + 4*4));
	}
	m_material.SetShadervalue("g_Alpha", {alpha_});
	m_material.SetShadervalue("g_Color", color_);

	int index = 0;
	for(auto& e:effects_){
		LOG_INFO("\n---Loading effect---");
		e.Load(wpRender);
	}
	fbo_ = std::unique_ptr<gl::GLFramebuffer>(wpRender.glWrapper.CreateFramebuffer(size_[0], size_[1]));
}

void ImageObject::Render(WPRender& wpRender)
{
	SetCurFbo(wpRender.GlobalFbo());
	
	wpRender.glWrapper.BindFramebuffer(fbo_.get());
	wpRender.glWrapper.Viewport(0,0,size_[0], size_[1]);

	if(copybackground_)	{
		wpRender.Clear(0.0f);
		if(Name() == "Compose") {
			wpRender.glWrapper.ActiveTexture(0);
			wpRender.glWrapper.BindTexture(&CurFbo()->color_texture);
		}
		glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
	}
	else {
		wpRender.Clear();
	}		

	SetCurVertices(&Vertices());
    m_material.Render(wpRender);


	glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
	SetCurFbo(fbo_.get());
	int index = 0;
	for(auto& e:effects_) {
		if(index++ == WallpaperGL::EffNum()) break;
		e.Render(wpRender);
	}

	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	wpRender.UseGlobalFbo(shadervalues_);
	wpRender.glWrapper.ActiveTexture(0);
	wpRender.glWrapper.BindTexture(&CurFbo()->color_texture);
	Vertices().Draw();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
