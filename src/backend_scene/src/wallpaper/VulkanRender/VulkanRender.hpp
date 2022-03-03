#pragma once

#include "Vulkan/Device.hpp"
#include "Vulkan/TextureCache.hpp"
#include "Vulkan/Swapchain.hpp"
#include "Vulkan/VulkanExSwapchain.hpp"

#include "RenderGraph/RenderGraph.hpp"

#include "SceneWallpaperSurface.hpp"
#include "VulkanPass.hpp"

#include <cstdio>
#include <memory>

namespace wallpaper
{
class Scene;

namespace vulkan
{

class VulkanRender {
public:

    bool init(Span<std::uint8_t> uuid);
    bool init(VulkanSurfaceInfo&);

    void destroy();

    void drawFrame(Scene&);
    void compileRenderGraph(Scene&, rg::RenderGraph&);
private:
    Instance m_instance;
    Swapchain m_swapchain;

    std::unique_ptr<Device> m_device;

    bool m_with_surface {false};
    bool m_inited       {false};

    std::unique_ptr<VulkanExSwapchain> m_ex_swapchain;
    std::array<RenderingResources, 3> m_rendering_resources;

    std::vector<VulkanPass*> m_passes;
};
}
}