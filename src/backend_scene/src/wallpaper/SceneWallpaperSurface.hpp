#pragma once
#include "SceneWallpaper.hpp"
#include "Utils/span.hpp"

#include <functional>
#include <string_view>
#include <vulkan/vulkan.h>

namespace wallpaper
{

struct VulkanSurfaceInfo {
    std::function<VkResult(VkInstance, VkSurfaceKHR*)> createSurfaceOp;
    std::vector<std::string> instanceExts;
};
}