#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace wallpaper
{

struct ShaderValue {
	std::string name;
	std::vector<float> value;

	static std::vector<float> ValueOf(const glm::mat4&);
	static std::vector<float> ValueOf(const glm::vec2&);
	static std::vector<float> ValueOf(const glm::vec4&);
};

typedef std::unordered_map<std::string, ShaderValue> ShaderValues;

struct ShaderAttribute {
public:
	std::string name;
	uint32_t location;
};


struct SceneShader {
public:
	uint32_t id;

	std::string vertexCode;
	std::string geometryCode;
	std::string fragmentCode;

	std::vector<ShaderAttribute> attrs;
	ShaderValues uniforms;
};
}
