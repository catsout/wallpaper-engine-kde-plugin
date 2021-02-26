#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <deque>
#include <chrono>
#include <ctime>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "GLWrapper.h"
#include "WPTextureCache.h"

namespace wallpaper
{
namespace gl
{

struct Shadervalue {
	std::string material;
	std::string glname;
	std::vector<float> value;
	std::string value_str;
	static void SetValue(Shadervalue&, const std::string&);
	static const std::string FindShadervalue(const std::unordered_map<std::string, Shadervalue>& shadervalues, const std::string& glname);
};

typedef std::unordered_map<std::string, int> Combos;
typedef std::unordered_map<std::string, Shadervalue> Shadervalues;

class GlobalUniform {
public:
	typedef void* (GlobalUniform::*value_func)();
	struct UniformValue {
		std::string name;
		void* value;
	};
	struct Camera {
		glm::vec3 center;
		glm::vec3 eye;
		glm::vec3 up;
	};
	GlobalUniform();
	void* GetValue(const std::string& name);
	bool IsGlobalUniform(const std::string& name);
	void SetCamera(std::vector<float>,std::vector<float>,std::vector<float>);
	std::vector<int> Ortho() {return ortho_;};
	void SetOrtho(int w, int h);
	void SetSize(int w, int h);
	void ClearCache() {cache_.clear();}
	bool CacheEmpty() {return cache_.empty();}

	void* Time();
	void* Daytime();
	void* TexelSize();
	void* TexelSizeHalf();
	void* ModelMatrix();
	void* ModelMatrixInverse();
	void* ViewProjectionMatrix();
	void* ModelViewProjectionMatrix();
	void* ModelViewProjectionMatrixInverse();
	const glm::mat4& GetViewProjectionMatrix();
private:
	Camera camera_;
	std::vector<int> ortho_;
	std::deque<UniformValue> cache_;
	typedef std::unordered_map<std::string, value_func> funcmap;
	funcmap funcmap_;
	std::chrono::time_point<std::chrono::steady_clock> startTime_;

	float time_;
	float daytime_;
	float alpha_;
	float size_[2];
	float halfsize_[2];
	glm::vec3 color_;
	glm::mat4 viewProjectionMatrix_;
	glm::mat4 modelMatrix_;
	glm::mat4 modelMatrixInverse_;
	glm::mat4 modelViewProjectionMatrix_;
	glm::mat4 modelViewProjectionMatrixInverse_;
};

class Shader {
public:
	Shader(GLWrapper* glWrapper, const std::string& name, GLuint stage, const std::string& source);
	~Shader();
	GLShader* shader;
	const std::string& Name() const {return name_;};
private:
	std::string name_;
	GLWrapper* glWrapper_;
};

struct WPShader {
	std::unique_ptr<Shader> vs;
	std::unique_ptr<Shader> fg;
	Combos combos;
	Shadervalues shadervalues;	
};

class LinkedShader {
public:
	LinkedShader(GLWrapper* glWrapper, Shader* vs, Shader* fg, const std::string& name);
	~LinkedShader();
	GLProgram* program;
	GLUniform* GetUniform(int index);
	std::vector<GLUniform>& GetUniforms() {return uniforms_;};
private:
	std::string name_;
	GLWrapper* glWrapper_;
	std::vector<GLShader*> shaders_;
	std::vector<GLUniform> uniforms_;
};

class WPShaderManager {
public:
	WPShaderManager(GLWrapper* glWrapper):glWrapper_(glWrapper) {};
	void CreateShader(const std::string& name, const Combos& combos, Shadervalues& shadervalues);
	void CreateShader(const std::string& name, const std::string& vsCode, const std::string& fgCode);
	LinkedShader* CreateLinkedShader(const std::string& name);
	void BindShader(const std::string& name);
	void UpdateUniforms(const std::string& name, const Shadervalues& shadervalues);
	void SetTextures(const std::string& name, Shadervalues& shadervalues);
	void ClearCache();
	GlobalUniform globalUniforms;
	static const std::string pre_shader_code;
private:
	Shader* CreateShader_(const std::string& name, GLuint stage, const std::string& source);

	GLWrapper* glWrapper_;
	typedef std::unordered_map<std::string, std::unique_ptr<LinkedShader>> LinkedShaderCache;
	LinkedShaderCache linkedCache_;	
	typedef std::unordered_map<std::string, WPShader> ShaderCache;
	ShaderCache shaderCache_;
};
}
}
