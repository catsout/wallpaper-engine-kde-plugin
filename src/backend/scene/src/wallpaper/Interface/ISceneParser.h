#pragma once
#include <string>
#include <memory>

#include "Scene.h"

namespace wallpaper
{

class ISceneParser {
public:
	ISceneParser() = default;
	virtual ~ISceneParser() = default;
	virtual std::unique_ptr<Scene> Parse(const std::string&) = 0;
};
}
