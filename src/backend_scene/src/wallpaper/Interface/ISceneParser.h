#pragma once
#include <string>
#include <memory>

#include "Scene/Scene.h"

namespace wallpaper
{

namespace fs{ class VFS; }
namespace audio{ class SoundManager; }
class ISceneParser {
public:
	ISceneParser() = default;
	virtual ~ISceneParser() = default;
	virtual std::shared_ptr<Scene> Parse(const std::string&, fs::VFS&, audio::SoundManager&) = 0;
};
}
