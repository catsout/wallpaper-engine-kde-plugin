#include "SceneShader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace wallpaper;

std::vector<float> ShaderValue::ValueOf(const glm::mat4& mat4) {
	const float* value = glm::value_ptr(mat4);
	return std::vector<float>(value, value + 16);
}

std::vector<float> ShaderValue::ValueOf(const glm::vec4& vec) {
	const float* value = glm::value_ptr(vec);
	return std::vector<float>(value, value + 4);
}

std::vector<float> ShaderValue::ValueOf(const glm::vec2& vec) {
	const float* value = glm::value_ptr(vec);
	return std::vector<float>(value, value + 2);
}
