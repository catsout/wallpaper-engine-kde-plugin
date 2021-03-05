#include <sstream>
#include <iostream>
#include "WPShaderManager.h"
#include "wallpaper.h"
#include "pkg.h"
#include "WPJson.h"
#include <ctype.h>

namespace wp = wallpaper;
using namespace wallpaper::gl;
using json = nlohmann::json;


void Shadervalue::SetValue(Shadervalue& sv, const std::string& value) {
	for(auto& c:value) {
		if(isalpha(c)) { 
			sv.value_str = value;
			return;
		}
	}
	wp::StringToVec<float>(value, sv.value);
}

void Shadervalue::SetShadervalues(Shadervalues& shadervalues, const std::string& glname, const std::vector<float>& value) {
	gl::Shadervalue sv = gl::Shadervalue();
	sv.glname = glname;
	sv.value = value;
	shadervalues[glname] = sv;
}

void Shadervalue::SetShadervalues(Shadervalues& shadervalues, const std::string& glname, const glm::mat4& value) {
	const float* value_ptr = glm::value_ptr(value);
	SetShadervalues(shadervalues, glname, std::vector<float>(value_ptr, value_ptr + 4*4));
}

GlobalUniform::GlobalUniform() {
	startTime_ = std::chrono::steady_clock::now();
	funcmap_["g_Time"] = &GlobalUniform::Time;
	funcmap_["g_Daytime"] = &GlobalUniform::Daytime;
	funcmap_["g_PointerPosition"] = &GlobalUniform::PointerPosition;
	funcmap_["g_TexelSize"] = &GlobalUniform::TexelSize;
	funcmap_["g_TexelSizeHalf"] = &GlobalUniform::TexelSizeHalf;
	funcmap_["g_ModelMatrix"] = &GlobalUniform::ModelMatrix;
	funcmap_["g_ModelMatrixInverse"] = &GlobalUniform::ModelMatrixInverse;
	funcmap_["g_ViewProjectionMatrix"] = &GlobalUniform::ViewProjectionMatrix;
	funcmap_["g_ModelViewProjectionMatrix"] = &GlobalUniform::ModelViewProjectionMatrix;
	funcmap_["g_ModelViewProjectionMatrixInverse"] = &GlobalUniform::ModelViewProjectionMatrixInverse;
	funcmap_["g_EffectTextureProjectionMatrix"] = &GlobalUniform::EffectTextureProjectionMatrix;
	funcmap_["g_EffectTextureProjectionMatrixInverse"] = &GlobalUniform::EffectTextureProjectionMatrixInverse;
	funcmap_["fboTrans"] = &GlobalUniform::FboTrans;
	camera_.center = glm::vec3(0.0f,0.0f,0.0f);
	camera_.eye = glm::vec3(0.0f,0.0f,1.0f);
	camera_.up = glm::vec3(0.0f,1.0f,0.0f);
	size_[0] = 1920.0f;
	size_[1] = 1080.0f;
	pointerPosition_[0] = 0.0f;
	pointerPosition_[1] = 0.0f;
}

bool GlobalUniform::IsGlobalUniform(const std::string& name) {
	return funcmap_.count(name) == 1;
}

void* GlobalUniform::GetValue(const std::string& name) {
	for(auto& el:cache_)
		if(name == el.name) {
			return el.value;
		}
	void* value = (this->*funcmap_[name])();
	cache_.push_back({name,value});
	return value;
}

void GlobalUniform::SetCamera(std::vector<float> center,std::vector<float> eye,std::vector<float> up) {
	camera_.center = glm::vec3(center[0],center[1],center[2]);
	camera_.eye = glm::vec3(eye[0],eye[1],eye[2]);
	camera_.up = glm::vec3(up[0],up[1],up[2]);
}

void GlobalUniform::SetOrtho(int w, int h) {
	ortho_ = {w,h};
}


void GlobalUniform::SetSize(int w, int h) {
	size_[0] = (float)w;
	size_[1] = (float)h;
}

void GlobalUniform::SetPointerPos(float x, float y) {
	pointerPosition_[0] = x;
	pointerPosition_[1] = y;
}

void* GlobalUniform::Time() {
	using namespace std::chrono;
	time_ = duration_cast<milliseconds>(steady_clock::now() - startTime_).count() / 1000.0f;
	if(time_ > 60.0f)
		startTime_ = steady_clock::now();
	return &time_;
}

void* GlobalUniform::Daytime() {
	using namespace std::chrono;
	daytime_ = duration_cast<minutes>(system_clock::now().time_since_epoch() % hours(24)).count() / (24.0f*60.0f);
	return &daytime_;
}

void* GlobalUniform::PointerPosition() {
	LOG_INFO(std::to_string(pointerPosition_[0]*100));
	return pointerPosition_;
}

void* GlobalUniform::TexelSize() {
	return size_;
}

void* GlobalUniform::TexelSizeHalf() {
	halfsize_[0] = size_[0]/2.0f;
	halfsize_[1] = size_[1]/2.0f;
	return halfsize_;
}

void* GlobalUniform::ModelMatrix() {
	modelMatrix_ = glm::mat4(1.0f);
	return glm::value_ptr(modelMatrix_);
}

void* GlobalUniform::ModelMatrixInverse() {
	GetValue("g_ModelMatrix");
	modelMatrixInverse_ = glm::inverse(modelMatrix_);
	return glm::value_ptr(modelMatrixInverse_);
}

void* GlobalUniform::ViewProjectionMatrix() {
//	auto view = glm::mat4(1.0f);
	auto view = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), camera_.up);
	viewProjectionMatrix_ = glm::ortho(0.0f, (float)ortho_[0], 0.0f, (float)ortho_[1], -100.0f, 100.0f)*view;
	return glm::value_ptr(viewProjectionMatrix_);
}

void* GlobalUniform::ModelViewProjectionMatrix() {
	GetValue("g_ViewProjectionMatrix");
	modelViewProjectionMatrix_ = viewProjectionMatrix_;
	return glm::value_ptr(modelViewProjectionMatrix_);
}

void* GlobalUniform::ModelViewProjectionMatrixInverse() {
	GetValue("g_ModelViewProjectionMatrix");
	modelViewProjectionMatrixInverse_ = glm::inverse(modelViewProjectionMatrix_);
	return glm::value_ptr(modelViewProjectionMatrixInverse_);
}

void* GlobalUniform::EffectTextureProjectionMatrix() {
	effectTextureProjectionMatrix_ = glm::mat4(1.0f);
	return glm::value_ptr(effectTextureProjectionMatrix_);
}

void* GlobalUniform::EffectTextureProjectionMatrixInverse() {
	GetValue("g_EffectTextureProjectionMatrix");
	effectTextureProjectionMatrixInverse_ = glm::inverse(effectTextureProjectionMatrix_);
	return glm::value_ptr(effectTextureProjectionMatrixInverse_);
}

void* GlobalUniform::FboTrans() {
	fboTrans_ = glm::mat4(1.0f);
	return glm::value_ptr(fboTrans_);
}

const glm::mat4& GlobalUniform::GetViewProjectionMatrix() {
	GetValue("g_ViewProjectionMatrix");
	return viewProjectionMatrix_;
}

LinkedShader::LinkedShader(GLWrapper* glWrapper, Shader* vs, Shader* fg, const std::string& name):glWrapper_(glWrapper),name_(name),uniforms_(1) {
	shaders_.push_back(vs->shader);
	shaders_.push_back(fg->shader);
	std::vector<GLProgram::AttribLoc> attrLocs;
	attrLocs.push_back({0,"a_Position"});
	attrLocs.push_back({1,"a_TexCoord"});
	program = glWrapper->CreateProgram(shaders_, attrLocs);
	glWrapper->GetUniforms(program, uniforms_);
}

GLUniform* LinkedShader::GetUniform(int index) {
	if(!uniforms_.size() > index)
		return &uniforms_[index];
	return nullptr;
}
LinkedShader::~LinkedShader() {
	glWrapper_->DeleteProgram(program);
}

Shader::Shader(GLWrapper* glWrapper, const std::string& name, GLuint stage, const std::string& source):glWrapper_(glWrapper),name_(name) {
	std::string output;
	shader = glWrapper->CreateShader(stage, source);
}

Shader::~Shader(){
	glWrapper_->DeleteShader(shader);
}

void LoadShaderWithInclude(std::string& source) 
{
	wp::LineStr line;
	std::string::size_type pos = 0;
	std::string find_str = "#include"; 
    while(line = wp::GetLineWith(source, find_str, pos),line.pos != std::string::npos)
    { 
		std::string new_src;
		std::stringstream line_stream(line.value);
		line_stream >> new_src; // drop #include
		line_stream >> new_src;
		new_src = new_src.substr(1, new_src.size()-2);  // drop "

		new_src = wp::fs::GetContent(wp::WallpaperGL::GetPkgfs(),"shaders/"+new_src);
		if(new_src.empty()) return;

		LoadShaderWithInclude(new_src);

		DeleteLine(source, line);
		source.insert(line.pos, new_src);
		pos = line.pos + new_src.size();
    }
}

std::string PreShaderSrc(GLenum shader_type, const std::string& src, Combos& combos, Shadervalues& shadervalues, int texcount) {
	std::string new_src = "";
	std::string include;
    std::string line;
    std::istringstream content(src);
	size_t last_var_pos = 0;
	
    while(std::getline(content,line))
    {
		bool update_pos = false;
		if(line.find("#include") != std::string::npos) {
			include.append(line + "\n");
			line = "";
		}
        else if(line.find("attribute ") != std::string::npos) {
            line = "in" + line.substr(sizeof("attribute")-1);
			update_pos = true;
		}
        else if(line.find("varying ") != std::string::npos) {
            line = (shader_type==GL_FRAGMENT_SHADER?"in":"out") + line.substr(sizeof("varying")-1);
			update_pos = true;
		}
		else if(line.find("// [COMBO]") != std::string::npos) {
			auto combo_json = json::parse(line.substr(line.find_first_of('{')));
			if(combo_json.contains("combo")) {
				std::string name;
				int value = 0;
				GET_JSON_NAME_VALUE(combo_json, "combo", name);
				GET_JSON_NAME_VALUE(combo_json, "default", value);
				combos[name] = value;
				line = "";
			}
		}
		else if(line.find("uniform ") != std::string::npos) {
			update_pos = true;
			if(line.find("// {") != std::string::npos) {
				auto sv_json = json::parse(line.substr(line.find_first_of('{')));
				std::vector<std::string> defines = wp::SpliteString(line.substr(0, line.find_first_of(';')), " ");
				Shadervalue sv;	
				sv.material = sv_json.at("material");
				sv.glname = defines.back();
				if(sv.glname[0] != 'g')
					LOG_INFO("PreShaderSrc User shadervalue not supported");
				if(sv_json.contains("default")){
					auto value = sv_json.at("default");
					if(value.is_string())
						Shadervalue::SetValue(sv, value);
                    if(value.is_number())
                        sv.value = std::vector<float>({value.get<float>()});
				/*
						GET_JSON_VALUE(value, sv.value);
					else {
						sv.value.resize(1);
						GET_JSON_VALUE(value, sv.value.at(0));
					}
				*/
				}
				shadervalues[sv.glname] = sv;
				if(sv_json.contains("combo")) {
					std::string name;
					int value = 1;
					GET_JSON_NAME_VALUE(sv_json, "combo", name);
					if(sv.glname.compare(0, 9, "g_Texture") == 0) {
						if(std::stoi(sv.glname.substr(9)) >= texcount)
							value = 0;
					}
					combos[name] = value;
				}
			}
		}
        new_src += line + '\n';
		if(update_pos)
			last_var_pos = new_src.size();
		if(line.find("void main()") != std::string::npos) {
			new_src += src.substr(content.tellg());
			break;
		}
	}
	if(new_src.substr(last_var_pos, 6) == "#endif")
		last_var_pos += 7;
	LoadShaderWithInclude(include);
    new_src.insert(last_var_pos, include);
	return new_src;
}


Shader* WPShaderManager::CreateShader_(const std::string& name, GLuint stage, const std::string& source) {
	auto& shader = shaderCache_[name];
	std::string source_header(WPShaderManager::pre_shader_code);
	for(auto& c:shader.combos) {
		source_header.append("#define " + c.first + " " + std::to_string(c.second) + "\n");
	}

	return new Shader(glWrapper_, name, stage, source_header + source);
}


std::string WPShaderManager::CreateShader(const std::string& name, const Combos& combos, Shadervalues& shadervalues, int texcount) {
	std::string shaderName;
	bool IsInCache = false;
	for(auto& el:shaderCache_) {
		if(el.first.compare(0, name.size(), name) == 0) {
			auto& shaderCombos = el.second.combos;
			auto& defaultCombos = el.second.combos;
			bool flag = true;
			for(auto& c:combos) {
				if(shaderCombos.count(c.first) == 0 || c.second != shaderCombos.at(c.first))
					flag = false;
			}
			// shadersCombos == active combos
			for(auto& c:shaderCombos) {
				if(combos.count(c.first) == 0) {
					if(defaultCombos.count(c.first) == 0 || defaultCombos.at(c.first) != c.second)
						flag = false;
				}
				else if(c.second != combos.at(c.first))
					flag = false;
			}
			if(flag) {
				IsInCache = true;
				shaderName = el.first;
			}
		}
	}
	if(!IsInCache || shaderName.empty()) {
		LOG_INFO("compile shader: " + name);
		std::string svCode = wp::fs::GetContent(wp::WallpaperGL::GetPkgfs(),"shaders/"+name+".vert");
		std::string fgCode = wp::fs::GetContent(wp::WallpaperGL::GetPkgfs(),"shaders/"+name+".frag");
		gl::Combos shaderCombos,defaultCombos;
		gl::Shadervalues shadervalues;
		svCode = PreShaderSrc(GL_VERTEX_SHADER, svCode, defaultCombos, shadervalues, texcount);
		fgCode = PreShaderSrc(GL_FRAGMENT_SHADER, fgCode, defaultCombos, shadervalues, texcount);
		shaderCombos = defaultCombos;

		for(const auto& c:combos) {
//			if(shaderCombos.count(c.first) != 0) 
				shaderCombos[c.first] = c.second;
		}
		shaderName = name+'+';
		for(auto& c:shaderCombos)
			shaderName.append(c.first + std::to_string(c.second));

		shaderCache_[shaderName] = WPShader();
		auto& shader = shaderCache_[shaderName];
		shader.combos = std::move(shaderCombos);
		shader.defaultCombos = std::move(defaultCombos);
		shader.shadervalues = std::move(shadervalues);


		shader.vs = std::unique_ptr<Shader>(CreateShader_(shaderName, GL_VERTEX_SHADER, svCode));
		shader.fg = std::unique_ptr<Shader>(CreateShader_(shaderName, GL_FRAGMENT_SHADER, fgCode));
	}
	for(auto el:shaderCache_[shaderName].shadervalues)
		shadervalues.insert(el);

	LOG_INFO("use shader: " + shaderName);
	return shaderName;
}


void WPShaderManager::CreateShader(const std::string& name, const std::string& vsCode, const std::string& fgCode) {
	if(!shaderCache_.count(name)) {
		shaderCache_[name] = WPShader();
		shaderCache_[name].vs = std::make_unique<Shader>(glWrapper_, name, GL_VERTEX_SHADER, vsCode);
		shaderCache_[name].fg = std::make_unique<Shader>(glWrapper_, name, GL_FRAGMENT_SHADER, fgCode);
	}
}

LinkedShader* WPShaderManager::CreateLinkedShader(const std::string& name) {
	if(!linkedCache_.count(name)) {
		Shader* vs = shaderCache_[name].vs.get();
		Shader* fg = shaderCache_[name].fg.get();
		linkedCache_[name] = std::unique_ptr<LinkedShader>(new LinkedShader(glWrapper_,vs ,fg , name));
		return linkedCache_[name].get();
	}
	return linkedCache_.at(name).get();
}


void WPShaderManager::BindShader(const std::string& name) {
	glWrapper_->BindProgram(linkedCache_[name]->program);
}

const std::string Shadervalue::FindShadervalue(const Shadervalues& shadervalues, const std::string& material) {
	for(const auto& sv:shadervalues)
		if(sv.second.material == material)
			return sv.first;
	return std::string();
}

void WPShaderManager::UpdateUniforms(const std::string& name, const Shadervalues& shadervalues) {
	auto& lkShader = linkedCache_[name];
	for(auto& u:lkShader->GetUniforms()) {
		const void* value = nullptr;
		if(shadervalues.count(u.name)) {
			auto& sv =  shadervalues.at(u.name);
			if(!sv.value.empty())
				value = &(sv.value[0]);
		}
		else if(globalUniforms.IsGlobalUniform(u.name))
			value = globalUniforms.GetValue(u.name);
//		if(value == nullptr)
//			LOG_INFO(std::string(" uniform: ") + u.name);
		if(value != nullptr){
//			std::cout << name << u.location << " uniform: "<< u.name << " --- " << *(const float*)value <<std::endl;
			glWrapper_->SetUniform(lkShader->program, &u, value);
		}
	}
}

// not load framebuffer tex
void WPShaderManager::SetTextures(const std::string& name, Shadervalues& shadervalues) {
	BindShader(name);	
	auto& lkShader = linkedCache_[name];
	for(auto& u:lkShader->GetUniforms()) {
		std::string name = std::string(u.name);
		if(name.compare(0, 9, "g_Texture") == 0 && name.size()<11) {
			int index = std::stoi(&u.name[9]);
			glWrapper_->SetUniformI(u.location, 1, &index);
			if(shadervalues.count(name) == 0) {
				Shadervalue sv;sv.glname = name;
				shadervalues[name] = sv;
			}
		}
	}
}

void WPShaderManager::ClearCache() {
	shaderCache_.clear();
	linkedCache_.clear();
}

void WPShaderManager::ClearShaderCache() {
	shaderCache_.clear();
}


const std::string WPShaderManager::pre_shader_code = "#version 330\n"
                                              "#define highp\n"
                                              "#define mediump\n"
                                              "#define lowp\n"
                                              "#define mul(x, y) (y * x)\n"
                                              "#define frac fract\n"
                                              "#define CAST2(x) (vec2(x))\n"
                                              "#define CAST3(x) (vec3(x))\n"
                                              "#define CAST4(x) (vec4(x))\n"
                                              "#define CAST3X3(x) (mat3(x))\n"
                                              "#define saturate(x) (clamp(x, 0.0, 1.0))\n"
                                              "#define texSample2D texture2D\n"
                                              "#define texSample2DLod texture2DLod\n"
                                              "#define texture2DLod texture2D\n"
                                              "#define atan2 atan\n"
                                              "#define ddx dFdx\n"
                                              //"#define VERSION\n"
											  "#define max(x, y) max(y, x)\n"
                                              "#define ddy(x) dFdy(-(x))\n\n";
