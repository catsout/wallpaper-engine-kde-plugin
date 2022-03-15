#pragma once
#include <unordered_map>

#include "SceneTexture.h"
#include "SceneRenderTarget.h"
#include "SceneNode.h"
#include "Interface/IShaderValueUpdater.h"
#include "Interface/IImageParser.h"
#include "Particle/ParticleSystem.h"
#include "Utils/NoCopyMove.hpp"
#include "Fs/VFS.h"


namespace wallpaper
{

namespace fs{
	class VFS;
}
class Scene : NoCopy,NoMove {
public:
	Scene():paritileSys(*this),
		sceneGraph(std::make_shared<SceneNode>()) {};
	~Scene() = default;

	std::unordered_map<std::string, SceneTexture> textures;
	std::unordered_map<std::string, SceneRenderTarget> renderTargets;

	std::unordered_map<std::string, std::shared_ptr<SceneCamera>> cameras;
	std::unordered_map<std::string, std::vector<std::string>> linkedCameras;

	std::shared_ptr<SceneNode> sceneGraph;	
	std::unique_ptr<IShaderValueUpdater> shaderValueUpdater;
	std::unique_ptr<IImageParser> imageParser;
	std::unique_ptr<fs::VFS> vfs;

	SceneMesh default_effect_mesh;

	ParticleSystem paritileSys;

	SceneCamera* activeCamera;

	uint16_t ortho[2] {1920, 1080}; // w, h
	std::array<float, 3> clearColor {1.0f, 1.0f, 1.0f};

	double elapsingTime {0.0f}, frameTime {0.0f};
	void PassFrameTime(double t) {
		frameTime = t;
		elapsingTime += t;
	}

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
