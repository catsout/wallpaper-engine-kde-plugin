#pragma once
#include "Interface/IShaderValueUpdater.h"

#include <memory>
#include <vector>
#include <unordered_map>

namespace wallpaper
{

class Scene;

struct WPResolution {
	int32_t index;
	float width;
	float height;
	float mapWidth;
	float mapHeight;
};

struct WPShaderValueData {
	std::vector<float> parallaxDepth {0.0f, 0.0f};
	std::vector<WPResolution> resolutions;
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
	void UpdateShaderValues(SceneNode*, SceneShader*) override;
	void FrameEnd() override;
	void MouseInput(double, double) override;

	void SetNodeData(void*, const WPShaderValueData&);
	void SetCameraParallax(const WPCameraParallax& value) { m_parallax = value; }
private:
	Scene* m_scene;	
	WPCameraParallax m_parallax;
	double m_lastTime {0.0f};
	double m_timeDiff {0.0f};
	std::vector<float> m_mousePos {0.5f, 0.5f};
	std::unordered_map<void*, WPShaderValueData> m_nodeDataMap;
};
}
