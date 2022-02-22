#pragma once

#include "Vulkan/Instance.hpp"
#include "Vulkan/TextureCache.hpp"
#include "Vulkan/Swapchain.hpp"
#include "Vulkan/VulkanExSwapchain.hpp"

#include "SceneWallpaperSurface.hpp"

#include <cstdio>
#include <memory>

namespace wallpaper
{
namespace vulkan
{

class VulkanRender {
public:

    bool init(Span<std::uint8_t> uuid);
    bool init(VulkanSurfaceInfo&);
private:
    Instance m_instance;
    Device m_device;
    Swapchain m_swapchain;

    bool m_with_surface {false};
    bool m_inited       {false};
    std::unique_ptr<TextureCache> m_tex_cache;

    std::unique_ptr<VulkanExSwapchain> m_ex_swapchain;
    std::array<RenderingResources, 3> m_rendering_resources;
};
}
}