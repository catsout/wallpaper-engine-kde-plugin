#include "VulkanRender.hpp"

#include "Utils/Logging.h"
#include <vector>

using namespace wallpaper::vulkan;

bool VulkanRender::init(Span<std::uint8_t> uuid) {
    return false;
}

bool VulkanRender::init(VulkanSurfaceInfo& info) {
    if(m_inited) return true;
    std::vector<const char*> inst_exts;
    std::transform(info.instanceExts.begin(), info.instanceExts.end(), std::back_insert_iterator(inst_exts), [](const auto& s){
        return s.c_str();
    });
    auto rv = Instance::Create(inst_exts);
    if(rv.result != vk::Result::eSuccess) {
        LOG_ERROR("init vulkan failed"); 
        return false;
    }
    m_instance = rv.value;
    vk::SurfaceKHR surface;
    if(info.createSurfaceOp(m_instance.inst(), (VkSurfaceKHR*)&surface) != VK_SUCCESS) {
        LOG_ERROR("create vulkan surface failed"); 
        return false;
    }
    m_instance.setSurface(surface);


    auto rv_device = Device::Create(m_instance, {VK_KHR_SWAPCHAIN_EXTENSION_NAME});
    if(rv_device.result != vk::Result::eSuccess) {
        LOG_ERROR("init vulkan device failed"); 
        return false;
    }
    m_device = rv_device.value;
    m_tex_cache = std::make_unique<TextureCache>(m_device);

    for(auto& rr:m_rendering_resources) {
        m_device.CreateRenderingResource(rr);
    }

    m_inited = true;
    m_with_surface = true;
    return m_inited;
}