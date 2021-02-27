#include <sstream>
#include <iostream>
#include "WPShaderManager.h"
#include "wallpaper.h"
#include "pkg.h"
#include <nlohmann/json.hpp>
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

GlobalUniform::GlobalUniform() {
	startTime_ = std::chrono::steady_clock::now();
	funcmap_["g_Time"] = &GlobalUniform::Time;
	funcmap_["g_Daytime"] = &GlobalUniform::Daytime;
	funcmap_["g_TexelSize"] = &GlobalUniform::TexelSize;
	funcmap_["g_TexelSizeHalf"] = &GlobalUniform::TexelSizeHalf;
	funcmap_["g_ModelMatrix"] = &GlobalUniform::ModelMatrix;
	funcmap_["g_ModelMatrixInverse"] = &GlobalUniform::ModelMatrixInverse;
	funcmap_["g_ViewProjectionMatrix"] = &GlobalUniform::ViewProjectionMatrix;
	funcmap_["g_ModelViewProjectionMatrix"] = &GlobalUniform::ModelViewProjectionMatrix;
	funcmap_["g_ModelViewProjectionMatrixInverse"] = &GlobalUniform::ModelViewProjectionMatrixInverse;
	funcmap_["fboTrans"] = &GlobalUniform::FboTrans;
	camera_.center = glm::vec3(0.0f,0.0f,0.0f);
	camera_.eye = glm::vec3(0.0f,0.0f,1.0f);
	camera_.up = glm::vec3(0.0f,1.0f,0.0f);
	size_[0] = 1920.0f;
	size_[1] = 1080.0f;
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
	auto view = glm::mat4(1.0f);
//	auto view = glm::lookAt(camera_.eye, camera_.center, camera_.up);
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

std::string PreShaderSrc(GLenum shader_type, const std::string& src, Combos& combos, Shadervalues& shadervalues) {
	std::string new_src = "";
	std::string include;
    std::string line;
    std::istringstream content(src);
	
    while(std::getline(content,line))
    {
		if(line.find("#include") != std::string::npos){
			include.append(line + "\n");
			line = "";
		}
        else if(line.find("attribute ") != std::string::npos)
            line = "in" + line.substr(sizeof("attribute")-1);

        else if(line.find("varying ") != std::string::npos)
            line = (shader_type==GL_FRAGMENT_SHADER?"in":"out") + line.substr(sizeof("varying")-1);

		else if(line.find("// [COMBO]") != std::string::npos) {
			auto combo_json = json::parse(line.substr(line.find_first_of('{')));
			if(combo_json.contains("combo")) {
				if(combos.count(combo_json.at("combo")))
					;
				else if(combo_json.contains("default")) {
					auto name = combo_json.at("combo").get<std::string>();
					int value = combo_json.at("default").get<int>();
					combos[name] = value;
				}
				line = "";
			}
		}

		else if(line.find("// {") != std::string::npos) {
			auto sv_json = json::parse(line.substr(line.find_first_of('{')));
			std::vector<std::string> defines = wp::SpliteString(line.substr(0, line.find_first_of(';')), " ");
			Shadervalue sv;	
			sv.material = sv_json.at("material");
			sv.glname = defines[2];
			if(sv.glname[0] != 'g')
				LOG_INFO("PreShaderSrc User shadervalue not supported");
			if(sv_json.contains("default")){
				auto value = sv_json.at("default");
				if(value.is_string())
					Shadervalue::SetValue(sv, value);
				if(value.is_number())
					sv.value = std::vector<float>({value.get<float>()});
			}
			shadervalues[sv.glname] = sv;
		}
        new_src += line + '\n';
	}
	LoadShaderWithInclude(include);
    new_src.insert(new_src.find("void main()"), include);
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


void WPShaderManager::CreateShader(const std::string& name, const Combos& combos, Shadervalues& shadervalues) {
	if(!shaderCache_.count(name)) {
		shaderCache_[name] = WPShader();
		auto& shader = shaderCache_[name];
		auto fileName = name.substr(0, name.find_first_of('+'));
		std::string svCode = wp::fs::GetContent(wp::WallpaperGL::GetPkgfs(),"shaders/"+fileName+".vert");
		std::string fgCode = wp::fs::GetContent(wp::WallpaperGL::GetPkgfs(),"shaders/"+fileName+".frag");
		shader.combos = combos;

		svCode = PreShaderSrc(GL_VERTEX_SHADER, svCode, shader.combos, shaderCache_[name].shadervalues);
		fgCode = PreShaderSrc(GL_FRAGMENT_SHADER, fgCode, shader.combos, shaderCache_[name].shadervalues);
		shader.vs = std::unique_ptr<Shader>(CreateShader_(name, GL_VERTEX_SHADER, svCode));
		shader.fg = std::unique_ptr<Shader>(CreateShader_(name, GL_FRAGMENT_SHADER, fgCode));
	}
	for(auto el:shaderCache_[name].shadervalues)
		shadervalues.insert(el);
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
                                              "#define MASK 1\n"
                                              //"#define VERSION\n"
											  "#define max(x, y) max(y, x)\n"
                                              "#define ddy(x) dFdy(-(x))\n\n";
