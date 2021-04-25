#pragma once
#include "SceneImageEffect.h"
#include <vector>
#include <list>

namespace wallpaper
{

class SceneNode;
class SceneImageEffectLayer {
public:
    SceneImageEffectLayer(SceneNode* node, float w, float h):m_worldNode(node) {};
    void AddEffect(const std::shared_ptr<SceneImageEffect>& node) {
        m_effects.push_back(node);
    }
    std::size_t EffectCount() const { return m_effects.size(); }
    auto& GetEffect(std::size_t index) { return m_effects.at(index); }
    const auto& FirstTarget() const { return m_firstTarget; }
    void SetFirstTarget(const std::string& ft) { m_firstTarget = ft; }

private:
    SceneNode* m_worldNode;
    std::string m_firstTarget;
    bool fullscreen {false};
//    std::vector<float> m_size;
    std::vector<std::shared_ptr<SceneImageEffect>> m_effects;
};
}
