#pragma once
#include <vector>
#include <list>
#include <memory>
#include <cstdint>
#include <string>


namespace wallpaper
{

class SceneNode;

class SceneNode;
struct SceneImageEffectNode {
    std::string output; // render target
    std::shared_ptr<SceneNode> sceneNode; 
};

struct SceneImageEffect {
    enum class CmdType {
        Copy,
    };
    struct Command {
        CmdType cmd {CmdType::Copy};
        std::string dst;
        std::string src;
        uint32_t afterpos {0}; // start at 1, 0 for begin at all
    };
    std::vector<Command> commands;
    std::list<SceneImageEffectNode> nodes; 
};

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
