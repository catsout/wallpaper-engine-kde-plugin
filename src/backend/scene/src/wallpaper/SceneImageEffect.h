#pragma once
#include <list>
#include <memory>
#include <cstdint>
#include <string>

namespace wallpaper
{

class SceneNode;
struct SceneImageEffectNode {
    std::string output; // render target
    std::shared_ptr<SceneNode> sceneNode; 
};

struct SceneImageEffect {
    std::list<SceneImageEffectNode> nodes; 
};
}