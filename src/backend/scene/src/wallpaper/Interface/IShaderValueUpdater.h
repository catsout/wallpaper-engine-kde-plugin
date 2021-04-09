#pragma once
//#include "SceneNode.h"


namespace wallpaper
{
class SceneNode;
class SceneShader;

class IShaderValueUpdater {
public:
	IShaderValueUpdater() = default;
	virtual ~IShaderValueUpdater() {};

	virtual void FrameBegin() = 0;
	virtual void UpdateShaderValues(SceneNode*, SceneShader*) = 0;
	virtual void FrameEnd() = 0;

	virtual void MouseInput(double x, double y) = 0;
	virtual void SetTexelSize(float x, float y) = 0;
};
}
