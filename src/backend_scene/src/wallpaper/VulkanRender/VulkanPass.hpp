#pragma once
#include "RenderGraph/Pass.hpp"

namespace wallpaper
{

class Scene;

namespace vulkan
{

class Device;
class RenderingResources;

class VulkanPass : public rg::Pass {
public:
    VulkanPass() = default;
    virtual ~VulkanPass() = default;
    virtual void prepare(Scene&, const Device&, RenderingResources&) = 0;
    virtual void execute(const Device&, RenderingResources&) = 0;
    virtual void destory(const Device&) = 0;

    bool prepared() const { return m_prepared; }
protected:
    void setPrepared() { m_prepared = true; }
private:
    bool m_prepared {false};
};
}
}