#pragma once
#include <string>
#include <vector>
#include <memory>

#include "SceneShader.h"
#include "Type.h"

namespace wallpaper
{

struct SceneMaterialCustomShader {
	std::shared_ptr<SceneShader> shader;
	std::vector<ShaderValue> updateValueList;
	ShaderValues constValues;
};

struct SceneMaterial {
public:
	SceneMaterial() = default;
	SceneMaterial(const SceneMaterial&) = default;
	SceneMaterial(SceneMaterial&& o)
		: name(std::move(o.name)),
		  textures(std::move(o.textures)),
		  defines(std::move(o.defines)) {};

	std::string name;
	std::vector<std::string> textures;
	std::vector<std::string> defines;

	bool hasSprite {false};

	SceneMaterialCustomShader customShader;
	BlendMode blenmode {BlendMode::Disable};
};
}
