#pragma once
#include <unordered_map>

#include "SceneTexture.h"
#include "SceneRenderTarget.h"
#include "SceneNode.h"
#include "Interface/IShaderValueUpdater.h"
#include "Interface/IImageParser.h"


namespace wallpaper
{

class Scene {
public:
	std::unordered_map<std::string, std::shared_ptr<SceneTexture>> textures;
	std::unordered_map<std::string, SceneRenderTarget> renderTargets;
	std::vector<std::string> fullScreenRenderTargets;

	std::unordered_map<std::string, std::shared_ptr<SceneCamera>> cameras;
	std::shared_ptr<SceneNode> sceneGraph;	
	std::unique_ptr<IShaderValueUpdater> shaderValueUpdater;
	std::unique_ptr<IImageParser> imageParser;

	SceneCamera* activeCamera;

	double elapsingTime {0.0f};
	std::vector<float> clearColor {1.0f, 1.0f, 1.0f};
};
}
