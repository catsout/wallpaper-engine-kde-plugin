#pragma once
#include <string>
#include <memory>

#include "Scene.h"

namespace wallpaper
{

class ISceneParser {
public:
	ISceneParser() {}
	virtual ~ISceneParser() {}
	virtual std::unique_ptr<Scene> Parse(const std::string&) = 0;
};
}
