#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <Eigen/Dense>

namespace wallpaper
{

struct ShaderValue {
	std::string name;
	std::vector<float> value;

	static inline std::vector<float> ValueOf(const Eigen::Ref<const Eigen::MatrixXf>& eig) {
		return std::vector<float>(eig.data(), eig.data() + eig.size());
	}
	static inline std::vector<float> ValueOf(const Eigen::Ref<const Eigen::MatrixXd>& eigd) {
		const Eigen::Ref<const Eigen::MatrixXf>& eig = eigd.cast<float>();
		return std::vector<float>(eig.data(), eig.data() + eig.size());
	}
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
