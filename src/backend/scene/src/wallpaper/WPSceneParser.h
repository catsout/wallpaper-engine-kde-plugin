#pragma once
#include "Interface/ISceneParser.h"

namespace wallpaper
{

class WPSceneParser : public ISceneParser {
public:
	WPSceneParser() = default;
	~WPSceneParser() = default;
	std::unique_ptr<Scene> Parse(const std::string&) override;
};
}
