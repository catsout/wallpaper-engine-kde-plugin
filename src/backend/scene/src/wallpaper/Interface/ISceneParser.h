#pragma once
#include <string>
#include <memory>

#include "Scene/Scene.h"

namespace wallpaper
{

namespace fs{
class VFS;
}
class ISceneParser {
public:
	ISceneParser() = default;
	virtual ~ISceneParser() = default;
	virtual std::unique_ptr<Scene> Parse(const std::string&, fs::VFS&) = 0;
};
}
