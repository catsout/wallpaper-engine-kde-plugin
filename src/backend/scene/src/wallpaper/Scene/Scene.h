#pragma once
#include <unordered_map>

#include "SceneTexture.h"
#include "SceneRenderTarget.h"
#include "SceneNode.h"
#include "Interface/IShaderValueUpdater.h"
#include "Interface/IImageParser.h"
#include "Particle/ParticleSystem.h"


namespace wallpaper
{

class Scene {
public:
	Scene():paritileSys(*this),
		sceneGraph(std::make_shared<SceneNode>()) {};
	~Scene() = default;
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
	Scene(Scene&&) = delete;
	Scene& operator=(Scene&&) = delete;

	std::unordered_map<std::string, SceneTexture> textures;
	std::unordered_map<std::string, SceneRenderTarget> renderTargets;

	std::unordered_map<std::string, std::shared_ptr<SceneCamera>> cameras;
	std::unordered_map<std::string, std::vector<std::string>> linkedCameras;

	std::shared_ptr<SceneNode> sceneGraph;	
	std::unique_ptr<IShaderValueUpdater> shaderValueUpdater;
	std::unique_ptr<IImageParser> imageParser;

	ParticleSystem paritileSys;

	SceneCamera* activeCamera;

	uint32_t ortho[2] {1920, 1080}; // w, h
	double elapsingTime {0.0f}, frameTime {0.0f};
	std::array<float, 3> clearColor {1.0f, 1.0f, 1.0f};

	void UpdateLinkedCamera(const std::string& name) {
		if(linkedCameras.count(name) != 0) {
			auto& cams = linkedCameras.at(name);
			for(auto& cam:cams) {
				if(cameras.count(cam) != 0) {
					cameras.at(cam)->Clone(*cameras.at(name));
					cameras.at(cam)->Update();
				}
			}
		}
	}
};
}
