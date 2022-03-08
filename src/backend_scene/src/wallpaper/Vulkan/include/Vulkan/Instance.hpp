#pragma once
#define VULKAN_HPP_NO_SMART_HANDLE
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>
#include <cassert>
#include "Utils/span.hpp"
#include "Utils/Logging.h"

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

class Device;
class Instance {
public:
    Instance() = default;
    ~Instance() = default;

    void Destroy();

    static bool Create(Instance&, Span<const char*const> instanceExts, Span<std::uint8_t> uuid={});

    const vk::Instance& inst() const;
    const vk::PhysicalDevice& gpu() const;
    const vk::SurfaceKHR& surface() const;
    bool offscreen() const;
    void setSurface(vk::SurfaceKHR);
private:
    vk::Instance m_inst;
    vk::DebugUtilsMessengerEXT m_debug_utils;
    vk::PhysicalDevice m_gpu;
    vk::SurfaceKHR m_surface;
};

}
}