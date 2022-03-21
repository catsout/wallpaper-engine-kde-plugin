#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_NO_SMART_HANDLE
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>
#include <cassert>
#include <string_view>
#include "Utils/span.hpp"
#include "Utils/Logging.h"
#include "Utils/MapSet.hpp"

#define VK_CHECK_RESULT(f) VK_CHECK_RESULT_ACT(, f)
#define VK_CHECK_RESULT_BOOL_RE(f) VK_CHECK_RESULT_ACT(return false, f)
#define VK_CHECK_RESULT_VOID_RE(f) VK_CHECK_RESULT_ACT(return, f)
#define VK_CHECK_RESULT_ACT(act,f)                                                                      \
{                                                                                                       \
    vk::Result res = (f);                                                                               \
    if (res != vk::Result::eSuccess)                                                                    \
    {                                                                                                   \
        LOG_ERROR("VkResult is \" %s \"", vk::to_string(res).c_str());                                  \
        assert(res == vk::Result::eSuccess);                                                            \
        {act;};                                                                                         \
    }                                                                                                   \
}


namespace wallpaper
{
namespace vulkan
{


struct Extension {
    bool required {false};
    std::string_view name;
};

using InstanceLayer = Extension;

constexpr std::string_view VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";

constexpr uint32_t WP_VULKAN_VERSION {VK_API_VERSION_1_1};
constexpr const char* WP_APPLICATION_NAME {"scene render"};

class Device;
class Instance {
public:
    Instance() = default;
    ~Instance() = default;

    void Destroy();

    static bool Create(Instance&, Span<Extension>, Span<InstanceLayer>, Span<std::uint8_t> uuid={});

    const vk::Instance& inst() const;
    const vk::PhysicalDevice& gpu() const;
    const vk::SurfaceKHR& surface() const;
    bool offscreen() const;
    void setSurface(vk::SurfaceKHR);
    bool supportExt(std::string_view) const;
    bool supportLayer(std::string_view) const;
private:
    vk::Instance m_inst;
    vk::DebugUtilsMessengerEXT m_debug_utils;
    vk::PhysicalDevice m_gpu;
    vk::SurfaceKHR m_surface;
    Set<std::string> m_extensions;
    Set<std::string> m_layers;
};

}
}