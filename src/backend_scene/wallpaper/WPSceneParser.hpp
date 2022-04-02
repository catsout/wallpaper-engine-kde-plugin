#pragma once
#include "Interface/ISceneParser.h"
#include <random>

namespace wallpaper
{

class WPSceneParser : public ISceneParser {
public:
    WPSceneParser()  = default;
    ~WPSceneParser() = default;
    std::shared_ptr<Scene> Parse(const std::string&, fs::VFS&, audio::SoundManager&) override;
};
} // namespace wallpaper
