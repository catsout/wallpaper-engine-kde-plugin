#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_NO_SMART_HANDLE
#define VULKAN_HPP_NO_EXCEPTIONS
// #include <vulkan/vulkan.hpp>
#include <cassert>
#include <string_view>
#include <span>
#include <functional>
#include "Core/Literals.hpp"
#include "Utils/Logging.h"
#include "Core/MapSet.hpp"

#include "../vvk/vulkan_wrapper.hpp"

namespace wallpaper
{
namespace vulkan
{

struct Extension {
    bool             required { false };
    std::string_view name;
};

using InstanceLayer = Extension;

using CheckGpuOp = std::function<bool(vvk::PhysicalDevice)>;

constexpr std::string_view VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";

constexpr uint32_t    WP_VULKAN_VERSION { VK_API_VERSION_1_1 };
constexpr const char* WP_APPLICATION_NAME { "scene render" };

class Device;
class Instance {
public:
    Instance()  = default;
    ~Instance() = default;

    void Destroy();

    static bool Create(Instance&, std::span<const Extension>, std::span<const InstanceLayer>);
    bool ChoosePhysicalDevice(const CheckGpuOp& checkgpu, std::span<const std::uint8_t> uuid = {});

    const vvk::Instance&       inst() const;
    const vvk::PhysicalDevice& gpu() const;
    const vvk::SurfaceKHR&     surface() const;

    bool offscreen() const;
    void setSurface(VkSurfaceKHR);
    bool supportExt(std::string_view) const;
    bool supportLayer(std::string_view) const;

private:
    utils::DynamicLibrary m_vklib;
    vvk::InstanceDispatch m_dld;
    vvk::Instance         m_vinst;

    vvk::DebugUtilsMessenger m_debug_utils;
    vvk::PhysicalDevice      m_gpu {};

    vvk::SurfaceKHR  m_surface {};
    Set<std::string> m_extensions;
    Set<std::string> m_layers;
};

} // namespace vulkan
} // namespace wallpaper
