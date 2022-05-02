#pragma once
#include "SceneWallpaper.hpp"
#include "Utils/span.hpp"

#include <functional>
#include <string_view>
#include <vulkan/vulkan.h>
#include <cstdint>

namespace wallpaper
{
using ReDrawCB = std::function<void()>;

struct VulkanSurfaceInfo {
    std::function<VkResult(VkInstance, VkSurfaceKHR*)> createSurfaceOp;
    std::vector<std::string>                           instanceExts;
};

struct RenderInitInfo {
    bool enable_valid_layer { false };
    bool offscreen { false };

    Span<const std::uint8_t> uuid;
    TexTiling                offscreen_tiling { TexTiling::OPTIMAL };
    VulkanSurfaceInfo        surface_info;

    uint16_t width { 1920 };
    uint16_t height { 1080 };
    ReDrawCB redraw_callback;
};

} // namespace wallpaper