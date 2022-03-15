#pragma once
#include "Interface/IShaderValueUpdater.h"
#include "Utils/MapSet.hpp"

#include <memory>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include "SpriteAnimation.h"
#include "WPPuppet.hpp"
#include <Eigen/Dense>

namespace wallpaper
{

class Scene;

struct WPShaderValueData {
	std::array<float, 2> parallaxDepth {0.0f, 0.0f};
	// index + name
	std::vector<std::pair<int32_t, std::string>> renderTargetResolution;

	std::vector<WPPuppet::AnimationLayer> puppet_layers;
	std::shared_ptr<WPPuppet> puppet;
};

struct WPCameraParallax {
	bool enable {false};
	float amount;
	float delay;
	float mouseinfluence;
};

class WPShaderValueUpdater : public IShaderValueUpdater {
public:
	WPShaderValueUpdater(Scene* scene):m_scene(scene) {}
	virtual ~WPShaderValueUpdater() {}
		
	void FrameBegin() override;
	void UpdateUniforms(SceneNode*, sprite_map_t&, const ExistsUniformOp&, const UpdateUniformOp&) override;
	void FrameEnd() override;
	void MouseInput(double, double) override;
	virtual void SetTexelSize(float x, float y) override;

	void SetNodeData(void*, const WPShaderValueData&);
	void SetCameraParallax(const WPCameraParallax& value) { m_parallax = value; }

	void SetOrtho(uint32_t w, uint32_t h) { m_ortho = {w, h}; }
private:
	Scene* m_scene;	
	WPCameraParallax m_parallax;
	double m_dayTime {0.0f};
	std::array<float, 2> m_texelSize {1.0f/1920.0f, 1.0f/1080.0f};
	std::array<float, 2> m_mousePos {0.5f, 0.5f};
	std::array<uint32_t, 2> m_ortho {1920, 1080};

	Map<void*, WPShaderValueData> m_nodeDataMap;
};
}
