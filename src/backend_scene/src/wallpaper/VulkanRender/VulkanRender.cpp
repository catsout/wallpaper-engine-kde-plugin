#include "VulkanRender.hpp"

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
        m_with_surface = true;
    }

    {
        m_device = std::make_unique<Device>();
        auto result = Device::Create(m_instance, std::array{
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME
        }, *m_device);
        if(result != vk::Result::eSuccess) {
            LOG_ERROR("init vulkan device failed"); 
            return false;
        }
    }

    initRes();
    for(auto& rr:m_rendering_resources) {
        CreateRenderingResource(rr);
    }

    m_inited = true;
    return m_inited;
}

vk::Result VulkanRender::initRes() {
    vk::Result res;
    do {
        m_finpass = std::make_unique<FinPass>(FinPass::Desc{});
        if(m_with_surface) {
            m_finpass->setPresentFormat(m_device->swapchain().format());
            m_finpass->setPresentQueueIndex(m_device->present_queue().family_index);
            m_finpass->setPresentLayout(vk::ImageLayout::ePresentSrcKHR);
        } else {
            m_finpass->setPresentFormat(m_ex_swapchain->format());
            m_finpass->setPresentQueueIndex(m_device->graphics_queue().family_index);
            m_finpass->setPresentLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        }

        m_vertex_buf = std::make_unique<StagingBuffer>(*m_device, 1024*1024,
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer
        );
        m_ubo_buf = std::make_unique<StagingBuffer>(*m_device, 4*1024*1024,
            vk::BufferUsageFlagBits::eUniformBuffer
        );
        (void)m_vertex_buf->allocate();
        res = m_ubo_buf->allocate();
        if(res != vk::Result::eSuccess) break;
        {
            vk::CommandPool pool = m_device->cmd_pool();
            auto rv = m_device->handle().allocateCommandBuffers({pool, vk::CommandBufferLevel::ePrimary, 1});
            res = rv.result;
            if(res != vk::Result::eSuccess) break;
            m_upload_cmd = rv.value.front();
        }
    } while(false);
    return res;
}

void VulkanRender::destroy() {
    (void)m_device->handle().waitIdle();

    // res
    m_vertex_buf->destroy();
    m_ubo_buf->destroy();
    m_device->handle().freeCommandBuffers(m_device->cmd_pool(), 1u, &m_upload_cmd);
    for(auto& rr:m_rendering_resources) {
        DestroyRenderingResource(rr);
    }
    m_device->Destroy();
    m_instance.Destroy();
}

vk::Result VulkanRender::CreateRenderingResource(RenderingResources& rr) {
	vk::Result result;
	do {
		auto rv_cmd = m_device->handle().allocateCommandBuffers({m_device->cmd_pool(), vk::CommandBufferLevel::ePrimary, 1});
		result = rv_cmd.result;
		if(result != vk::Result::eSuccess) break;
		rr.command = rv_cmd.value.front();

		auto rv_f = m_device->handle().createFence({vk::FenceCreateFlagBits::eSignaled});
		result = rv_f.result;
		if(result != vk::Result::eSuccess) break;
		rr.fence_frame = rv_f.value;
		result = m_device->handle().resetFences(1, &rr.fence_frame);

        if(m_with_surface) {
            vk::SemaphoreCreateInfo info;
            (void)m_device->handle().createSemaphore(&info, nullptr, &rr.sem_swap_finish);
            result = m_device->handle().createSemaphore(&info, nullptr, &rr.sem_swap_wait_image);
            if(result != vk::Result::eSuccess) break;
        }

        rr.vertex_buf = m_vertex_buf.get();
        rr.ubo_buf = m_ubo_buf.get();
	} while(false);
	return result;
}


void VulkanRender::DestroyRenderingResource(RenderingResources& rr) {
	m_device->handle().freeCommandBuffers(m_device->cmd_pool(), 1, &rr.command);
	m_device->handle().destroyFence(rr.fence_frame);
    if(rr.sem_swap_finish) {
        m_device->handle().destroySemaphore(rr.sem_swap_finish);
        m_device->handle().destroySemaphore(rr.sem_swap_wait_image);
    }
}


void VulkanRender::drawFrame(Scene& scene) {
    if(!m_inited) return;
    if(m_instance.offscreen()) {
        drawFrameOffscreen();
    } else {
        drawFrameSwapchain();
    }
}

void VulkanRender::drawFrameSwapchain() {
    static size_t resource_index = 0;

    RenderingResources& rr = m_rendering_resources[0];
    resource_index = (resource_index + 1) % 3;
    uint32_t image_index = m_device->handle().acquireNextImageKHR(m_device->swapchain().handle(), UINT32_MAX, rr.sem_swap_wait_image, {}).value;
    const auto& image = m_device->swapchain().images()[image_index];

    (void)m_device->handle().waitForFences(1, &rr.fence_frame, false, INT_MAX);
    (void)m_device->handle().resetFences(1, &rr.fence_frame);

    m_finpass->setPresent(image);

    (void)rr.command.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    m_ubo_buf->recordUpload(rr.command);
    for(auto* p:m_passes) {
        if(p->prepared()) {
            p->execute(*m_device, rr);
        }
    }
    (void)rr.command.end();

    vk::PipelineStageFlags wait_dst_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo sub_info;
    sub_info.setCommandBufferCount(1)
        .setPCommandBuffers(&rr.command)
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(&rr.sem_swap_wait_image)
        .setSignalSemaphoreCount(1)
        .setPSignalSemaphores(&rr.sem_swap_finish)
        .setPWaitDstStageMask(&wait_dst_stage);
    (void)m_device->present_queue().handle.submit(1, &sub_info, rr.fence_frame);
    vk::PresentInfoKHR present_info;
    present_info.setSwapchainCount(1)
        .setPSwapchains(&m_device->swapchain().handle())
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(&rr.sem_swap_finish)
        .setPImageIndices(&image_index);
    (void)m_device->present_queue().handle.presentKHR(present_info);
}
void VulkanRender::drawFrameOffscreen() {

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
    m_passes.push_back(m_finpass.get());
    if(m_with_surface) {
        m_device->set_out_extent(m_device->swapchain().extent());
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
    (void)m_upload_cmd.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    m_vertex_buf->recordUpload(m_upload_cmd);
    (void)m_upload_cmd.end();
    {
    	vk::SubmitInfo sub_info;
		sub_info.setCommandBufferCount(1)
			.setPCommandBuffers(&m_upload_cmd);
		(void)m_device->graphics_queue().handle.submit(1, &sub_info, {});
        (void)m_device->handle().waitIdle();
    }
};