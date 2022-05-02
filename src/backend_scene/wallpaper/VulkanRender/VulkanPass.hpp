#pragma once
#include "RenderGraph/Pass.hpp"
#include "Utils/span.hpp"
#include <string_view>

namespace wallpaper
{

class Scene;

namespace vulkan
{

class Device;
class RenderingResources;
class Resource;

class VulkanPass : public rg::Pass {
public:
    VulkanPass() = default;
    virtual ~VulkanPass() = default;
    virtual void prepare(Scene&, const Device&, RenderingResources&) = 0;
    virtual void execute(const Device&, RenderingResources&) = 0;
    virtual void destory(const Device&, RenderingResources&) = 0;

    void addReleaseTexs(Span<const std::string_view> texs) {
        m_release_texs.clear();
        std::transform(texs.begin(), texs.end(), std::back_inserter(m_release_texs), [](auto& sv) {
            return std::string(sv);
        });
    }
    bool prepared() const { return m_prepared; }
    Span<const std::string> releaseTexs() const { return m_release_texs; }
    void clearReleaseTexs() { m_release_texs.clear(); }
protected:
    void setPrepared(bool v=true) { m_prepared = v; }
private:
    bool m_prepared {false};
    std::vector<std::string> m_release_texs;
};
}
}