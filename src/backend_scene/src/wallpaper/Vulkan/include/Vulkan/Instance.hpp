#pragma once
#define VULKAN_HPP_NO_SMART_HANDLE
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>
#include "Utils/span.hpp"

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

    static vk::ResultValue<Instance> Create(Span<const char*const> instanceExts, Span<std::uint8_t> uuid={});

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