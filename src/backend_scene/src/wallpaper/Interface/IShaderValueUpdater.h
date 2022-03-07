#pragma once
#include "Utils/NoCopyMove.hpp"
#include <functional>
#include <string_view>

namespace wallpaper
{
class SceneNode;
class SceneShader;
class ShaderValue;

using UpdateUniformOp = std::function<void(std::string_view, ShaderValue)>;
using ExistsUniformOp = std::function<bool(std::string_view)>;

class IShaderValueUpdater : NoCopy,NoMove {
public:
	IShaderValueUpdater() = default;
	virtual ~IShaderValueUpdater() = default;

	virtual void FrameBegin() = 0;
	virtual void UpdateUniforms(SceneNode*, const ExistsUniformOp&, const UpdateUniformOp&) = 0;
	virtual void FrameEnd() = 0;

	virtual void MouseInput(double x, double y) = 0;
	virtual void SetTexelSize(float x, float y) = 0;
};
}
