#include "VulkanRender.hpp"

#include "VulkanPass.hpp"

#include "Utils/Logging.h"
#include "RenderGraph/RenderGraph.hpp"
#include "Scene/Scene.h"

#include <glslang/Public/ShaderLang.h>

#include <cassert>
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
    {
        VkSurfaceKHR surface;
        if(info.createSurfaceOp(m_instance.inst(), &surface) != VK_SUCCESS) {
            LOG_ERROR("create vulkan surface failed"); 
            return false;
        }
        m_instance.setSurface(vk::SurfaceKHR(surface));
    }

    {
        m_device = std::make_unique<Device>();
        auto result = Device::Create(m_instance, {VK_KHR_SWAPCHAIN_EXTENSION_NAME}, *m_device);
        if(result != vk::Result::eSuccess) {
            LOG_ERROR("init vulkan device failed"); 
            return false;
        }
    }

    for(auto& rr:m_rendering_resources) {
        m_device->CreateRenderingResource(rr);
    }

    m_inited = true;
    m_with_surface = true;
    return m_inited;
}

void VulkanRender::destroy() {
    for(auto& rr:m_rendering_resources) {
        m_device->DestroyRenderingResource(rr);
    }
    m_device->Destroy();
    m_instance.Destroy();
}

void VulkanRender::drawFrame(Scene& scene) {


}

void VulkanRender::compileRenderGraph(Scene& scene, rg::RenderGraph& rg) {
    auto nodes = rg.topologicalOrder();
    m_passes.clear();
    m_passes.resize(nodes.size());
    std::transform(nodes.begin(), nodes.end(), m_passes.begin(), [&rg](auto id) {
        auto* pass = rg.getPass(id);
        assert(pass != nullptr);
        VulkanPass* vpass = static_cast<VulkanPass*>(pass);
        return vpass;
    });
    if(m_with_surface) {
        m_device->set_out_extent(m_swapchain.extent());
    }
    for(auto& item:scene.renderTargets) {
        auto& rt = item.second;
        if(rt.bind.enable && rt.bind.screen) {
            rt.width = rt.bind.scale * m_device->out_extent().width;
            rt.height = rt.bind.scale * m_device->out_extent().height;
        }
    }
    for(auto& item:scene.renderTargets) {
        auto& rt = item.second;
		if(rt.bind.screen) continue;
		auto bind_rt = scene.renderTargets.find(rt.bind.name);
		if(bind_rt == scene.renderTargets.end()) continue;
		rt.width = rt.bind.scale * bind_rt->second.width;
		rt.height = rt.bind.scale * bind_rt->second.height;
    }
    glslang::InitializeProcess();
    for(auto* p:m_passes) {
        if(!p->prepared()) {
            p->prepare(scene, *m_device, m_rendering_resources[0]);
        }
    }
    glslang::FinalizeProcess();
};