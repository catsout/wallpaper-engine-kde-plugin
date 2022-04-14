#include "VulkanRender.hpp"

#include "Utils/Logging.h"
#include "RenderGraph/RenderGraph.hpp"
#include "Scene/Scene.h"
#include "Utils/Algorism.h"

#include <glslang/Public/ShaderLang.h>

#include <cassert>
#include <vector>
#include <cstdint>


#if ENABLE_RENDERDOC_API
#include "RenderDoc.h"
#endif

using namespace wallpaper::vulkan;

constexpr uint64_t vk_wait_time {10u*1000u*1000000u};

constexpr std::array base_inst_exts {
    Extension { false, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME },
};
constexpr std::array base_device_exts {
    Extension { false, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME },
    Extension { true, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME },
    Extension { true, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME },
    Extension { true, VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME },
    Extension { true, VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME },
    Extension { true, VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME }
};


bool VulkanRender::inited() const { return m_inited; }


bool VulkanRender::init(RenderInitInfo info) {
    if(m_inited) return true;

    m_redraw_cb = info.redraw_callback;
    vk::Extent2D extent {info.width,info.height};
    if(extent.width * extent.height < 500*500) {
        LOG_ERROR("too small swapchain image size: %dx%d", extent.width, extent.height);
    } else {
        LOG_INFO("set swapchain image size: %dx%d", extent.width, extent.height);
    }

    std::vector<Extension> inst_exts {base_inst_exts.begin(), base_inst_exts.end()};
    std::vector<Extension> device_exts {base_device_exts.begin(), base_device_exts.end()};

    if(!info.offscreen) {
        std::transform(info.surface_info.instanceExts.begin(), info.surface_info.instanceExts.end(), std::back_inserter(inst_exts), [](const auto& s){
            return Extension{true, s.c_str()};
        });
        device_exts.push_back({ true, VK_KHR_SWAPCHAIN_EXTENSION_NAME });
    }

    std::vector<InstanceLayer> inst_layers;
    // valid layer
    if(info.enable_valid_layer) {
        inst_layers.push_back({true, VALIDATION_LAYER_NAME});
        LOG_INFO("vulkan valid layer \"%s\" enabled", VALIDATION_LAYER_NAME.data());
    }

    if(!Instance::Create(m_instance, inst_exts, inst_layers)) {
        LOG_ERROR("init vulkan failed"); 
        return false;
    }
    if(!info.offscreen){
        VkSurfaceKHR surface;
        VK_CHECK_RESULT_ACT({
            LOG_ERROR("create vulkan surface failed"); 
            return false;
        }, (vk::Result)info.surface_info.createSurfaceOp(m_instance.inst(), &surface));
        m_instance.setSurface(vk::SurfaceKHR(surface));
        m_with_surface = true;
    }
    {
        auto surface = m_instance.surface();
        auto check_gpu = [&info, &device_exts, surface](vk::PhysicalDevice gpu) {
            return Device::CheckGPU(gpu, device_exts, surface);
        };
        if(!m_instance.ChoosePhysicalDevice(check_gpu, info.uuid))
            return false;
    }

    {
        m_device = std::make_unique<Device>();
        if(!Device::Create(m_instance, device_exts, extent, *m_device)) {
            LOG_ERROR("init vulkan device failed"); 
            return false;
        }
    }

    if(info.offscreen) {
        m_ex_swapchain = CreateExSwapchain(*m_device, extent.width, extent.height, (info.offscreen_tiling == TexTiling::OPTIMAL 
            ? vk::ImageTiling::eOptimal 
            : vk::ImageTiling::eLinear));
        m_with_surface = false;
    }

    if(!initRes()) return false;;

    m_inited = true;
    return m_inited;
}

bool VulkanRender::initRes() {
    m_prepass = std::make_unique<PrePass>(PrePass::Desc{});
    m_finpass = std::make_unique<FinPass>(FinPass::Desc{});
    if(m_with_surface) {
        m_finpass->setPresentFormat(m_device->swapchain().format());
        m_finpass->setPresentQueueIndex(m_device->present_queue().family_index);
        m_finpass->setPresentLayout(vk::ImageLayout::ePresentSrcKHR);
    } else {
        m_finpass->setPresentFormat(m_ex_swapchain->format());
        m_finpass->setPresentLayout(vk::ImageLayout::eGeneral);
        m_finpass->setPresentQueueIndex(VK_QUEUE_FAMILY_EXTERNAL);
    }
    /*
    m_testpass = std::make_unique<FinPass>(FinPass::Desc{});
    m_testpass->setPresentFormat(m_ex_swapchain->format());
    m_testpass->setPresentQueueIndex(m_device->graphics_queue().family_index);
    m_testpass->setPresentLayout(vk::ImageLayout::ePresentSrcKHR);
    */

    m_vertex_buf = std::make_unique<StagingBuffer>(*m_device, 2*1024*1024,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer
    );
    m_dyn_buf = std::make_unique<StagingBuffer>(*m_device, 2*1024*1024,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eUniformBuffer
    );
    if(!m_vertex_buf->allocate()) return false;
    if(!m_dyn_buf->allocate()) return false;
    {
        vk::CommandPool pool = m_device->cmd_pool();
        auto rv = m_device->handle().allocateCommandBuffers({pool, vk::CommandBufferLevel::ePrimary, 1});
        VK_CHECK_RESULT_BOOL_RE(rv.result);
        m_upload_cmd = rv.value.front();
    }
    for(auto& rr:m_rendering_resources) {
        if(!CreateRenderingResource(rr)) return false;
    }

#if ENABLE_RENDERDOC_API
    load_renderdoc_api();
#endif
    return true;
}

void VulkanRender::destroy() {
    if(!m_inited) return;
    if(m_device && m_device->handle()) {
        VK_CHECK_RESULT(m_device->handle().waitIdle());

        // res
        for(auto& p:m_passes) {
            p->destory(*m_device, m_rendering_resources[0]);
        }
        m_vertex_buf->destroy();
        m_dyn_buf->destroy();
        m_device->handle().freeCommandBuffers(m_device->cmd_pool(), 1u, &m_upload_cmd);
        for(auto& rr:m_rendering_resources) {
            DestroyRenderingResource(rr);
        }
        if(m_ex_swapchain) {
            for(auto& exh:m_ex_swapchain->handles()) {
                m_device->DestroyExImageParameters(exh.image);
            }
        }
        m_device->Destroy();
    }
    m_instance.Destroy();
}

bool VulkanRender::CreateRenderingResource(RenderingResources& rr) {
    auto rv_cmd = m_device->handle().allocateCommandBuffers({m_device->cmd_pool(), vk::CommandBufferLevel::ePrimary, 1});
    VK_CHECK_RESULT_BOOL_RE(rv_cmd.result);
    rr.command = rv_cmd.value.front();

    auto rv_f = m_device->handle().createFence({vk::FenceCreateFlagBits::eSignaled});
    VK_CHECK_RESULT_BOOL_RE(rv_f.result);
    rr.fence_frame = rv_f.value;

    VK_CHECK_RESULT_BOOL_RE(m_device->handle().resetFences(1, &rr.fence_frame));

    if(m_with_surface) {
        vk::SemaphoreCreateInfo info;
        VK_CHECK_RESULT_BOOL_RE(m_device->handle().createSemaphore(&info, nullptr, &rr.sem_swap_finish));
        VK_CHECK_RESULT_BOOL_RE(m_device->handle().createSemaphore(&info, nullptr, &rr.sem_swap_wait_image));
    }

    rr.vertex_buf = m_vertex_buf.get();
    rr.dyn_buf = m_dyn_buf.get();
	return true;
}


void VulkanRender::DestroyRenderingResource(RenderingResources& rr) {
	m_device->handle().freeCommandBuffers(m_device->cmd_pool(), 1, &rr.command);
	m_device->handle().destroyFence(rr.fence_frame);
    if(rr.sem_swap_finish) {
        m_device->handle().destroySemaphore(rr.sem_swap_finish);
        m_device->handle().destroySemaphore(rr.sem_swap_wait_image);
    }
}

VulkanExSwapchain* VulkanRender::exSwapchain() const {
    return m_ex_swapchain.get();
}

void VulkanRender::drawFrame(Scene& scene) {
    if(!(m_inited && m_pass_loaded)) return;

    //LOG_INFO("used ram: %fm", (m_device->GetUsage()/1024.0f)/1024.0f);

#if ENABLE_RENDERDOC_API
    if(rdoc_api)
        rdoc_api->StartFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE((VkInstance)m_instance.inst()), NULL);
#endif

    if(m_instance.offscreen()) {
        drawFrameOffscreen();
    } else {
        drawFrameSwapchain();
    }

    if(m_redraw_cb) m_redraw_cb();

#if ENABLE_RENDERDOC_API
    if(rdoc_api)
        rdoc_api->EndFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE((VkInstance)m_instance.inst()),NULL);
#endif
}

void VulkanRender::drawFrameSwapchain() {
    static size_t resource_index = 0;

    RenderingResources& rr = m_rendering_resources[0];
    resource_index = (resource_index + 1) % 3;
    uint32_t image_index = 0;
    {
        auto rv = m_device->handle().acquireNextImageKHR(m_device->swapchain().handle(), vk_wait_time, rr.sem_swap_wait_image, {});
        VK_CHECK_RESULT_VOID_RE(rv.result);
        image_index = rv.value;
    }
    const auto& image = m_device->swapchain().images()[image_index];


    m_finpass->setPresent(image);

    VK_CHECK_RESULT_VOID_RE(rr.command.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit}));
    m_dyn_buf->recordUpload(rr.command);
    for(auto* p:m_passes) {
        if(p->prepared()) {
            p->execute(*m_device, rr);
        }
    }
    VK_CHECK_RESULT_VOID_RE(rr.command.end());

    vk::PipelineStageFlags wait_dst_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo sub_info;
    sub_info.setCommandBufferCount(1)
        .setPCommandBuffers(&rr.command)
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(&rr.sem_swap_wait_image)
        .setSignalSemaphoreCount(1)
        .setPSignalSemaphores(&rr.sem_swap_finish)
        .setPWaitDstStageMask(&wait_dst_stage);
    VK_CHECK_RESULT_VOID_RE(m_device->present_queue().handle.submit(1, &sub_info, rr.fence_frame));
    vk::PresentInfoKHR present_info;
    present_info.setSwapchainCount(1)
        .setPSwapchains(&m_device->swapchain().handle())
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(&rr.sem_swap_finish)
        .setPImageIndices(&image_index);
    VK_CHECK_RESULT_VOID_RE(m_device->present_queue().handle.presentKHR(present_info));

    VK_CHECK_RESULT_VOID_RE(m_device->handle().waitForFences(1, &rr.fence_frame, false, vk_wait_time));
    VK_CHECK_RESULT_VOID_RE(m_device->handle().resetFences(1, &rr.fence_frame));
}
void VulkanRender::drawFrameOffscreen() {
    RenderingResources& rr = m_rendering_resources[0];
    ImageParameters image = m_ex_swapchain->GetInprogressImage().toImageParameters();

    m_finpass->setPresent(image);

    VK_CHECK_RESULT_VOID_RE(rr.command.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit}));

    m_dyn_buf->recordUpload(rr.command);

    for(auto* p:m_passes) {
        if(p->prepared()) {
            p->execute(*m_device, rr);
        }
    }

    VK_CHECK_RESULT_VOID_RE(rr.command.end());

    vk::SubmitInfo sub_info;
    sub_info.setCommandBufferCount(1)
        .setPCommandBuffers(&rr.command);
    VK_CHECK_RESULT_VOID_RE(m_device->graphics_queue().handle.submit(1, &sub_info, rr.fence_frame));

    VK_CHECK_RESULT_VOID_RE(m_device->handle().waitForFences(1, &rr.fence_frame, false, vk_wait_time));
    VK_CHECK_RESULT_VOID_RE(m_device->handle().resetFences(1, &rr.fence_frame));
    m_ex_swapchain->renderFrame();
}

void VulkanRender::setRenderTargetSize(Scene& scene, rg::RenderGraph& rg) {
    auto& ext = m_device->out_extent();
    for(auto& item:scene.renderTargets) {
        auto& rt = item.second;
        if(rt.bind.enable && rt.bind.screen) {
            rt.width = rt.bind.scale * ext.width;
            rt.height = rt.bind.scale * ext.height;
        }
    }
    for(auto& item:scene.renderTargets) {
        auto& rt = item.second;
		if(rt.bind.screen || !rt.bind.enable) continue;
		auto bind_rt = scene.renderTargets.find(rt.bind.name);
		if(rt.bind.name.empty() || bind_rt == scene.renderTargets.end()) {
            LOG_ERROR("unknonw render target bind: %s", rt.bind.name.c_str());
            continue;
        }
		rt.width = rt.bind.scale * bind_rt->second.width;
		rt.height = rt.bind.scale * bind_rt->second.height;
    }
    for(auto& item:scene.renderTargets) {
        auto& rt = item.second;
		if(!item.first.empty() && (rt.width*rt.height <= 4)) {
            LOG_ERROR("wrong size for render target: %s", item.first.c_str());
        } else if(rt.has_mipmap) {
            rt.mipmap_level = std::max(3u, static_cast<uint>(std::floor(std::log2(std::min(rt.width, rt.height))))) - 2u;
        }
    }
    scene.shaderValueUpdater->SetScreenSize(ext.width, ext.height);
}

void VulkanRender::UpdateCameraFillMode(wallpaper::Scene& scene, wallpaper::FillMode fillmode) {
    using namespace wallpaper;
    auto width = m_device->out_extent().width;
    auto height = m_device->out_extent().height;

    if(width == 0) return;
	double sw = scene.ortho[0],sh = scene.ortho[1];
	double fboAspect = width/(double)height, sAspect = sw/sh;
	auto& gCam = *scene.cameras.at("global");
	auto& gPerCam = *scene.cameras.at("global_perspective");
	// assum cam 
	switch (fillmode)
	{
	case FillMode::STRETCH:
		gCam.SetWidth(sw);
		gCam.SetHeight(sh);
		gPerCam.SetAspect(sAspect);
		gPerCam.SetFov(algorism::CalculatePersperctiveFov(1000.0f, gCam.Height()));
		break;
	case FillMode::ASPECTFIT:
		if(fboAspect < sAspect) {
			// scale height
			gCam.SetWidth(sw);
			gCam.SetHeight(sw / fboAspect);
		} else {
			gCam.SetWidth(sh * fboAspect);
			gCam.SetHeight(sh);
		}
		gPerCam.SetAspect(fboAspect);
		gPerCam.SetFov(algorism::CalculatePersperctiveFov(1000.0f, gCam.Height()));
		break;
	case FillMode::ASPECTCROP:
	default:
		if(fboAspect > sAspect) {
			// scale height
			gCam.SetWidth(sw);
			gCam.SetHeight(sw / fboAspect);
		} else {
			gCam.SetWidth(sh * fboAspect);
			gCam.SetHeight(sh);
		}
		gPerCam.SetAspect(fboAspect);
		gPerCam.SetFov(algorism::CalculatePersperctiveFov(1000.0f, gCam.Height()));
		break;
	}
	gCam.Update();
	gPerCam.Update();
	scene.UpdateLinkedCamera("global");
}


void VulkanRender::clearLastRenderGraph() {
    for(auto& p:m_passes) {
        p->destory(*m_device, m_rendering_resources[0]);
    }
    m_passes.clear();
    m_device->tex_cache().Clear();

    m_vertex_buf->destroy();
    m_dyn_buf->destroy();

    m_vertex_buf->allocate();
    m_dyn_buf->allocate();
}

void VulkanRender::compileRenderGraph(Scene& scene, rg::RenderGraph& rg) {
    if(!m_inited) return;
    m_pass_loaded = false;
    clearLastRenderGraph();

    auto nodes = rg.topologicalOrder();
    auto node_release_texs = rg.getLastReadTexs(nodes);

    m_passes.clear();
    m_passes.resize(nodes.size());
    
    std::transform(nodes.begin(), nodes.end(), node_release_texs.begin(), m_passes.begin(), [&rg](auto& id, auto& texs) {
        auto* pass = rg.getPass(id);
        assert(pass != nullptr);
        VulkanPass* vpass = static_cast<VulkanPass*>(pass);
        //LOG_INFO("----release tex");
        for(auto& tex:texs) { 
            vpass->addReleaseTexs(tex->key());
        //    LOG_INFO("%s", tex->key().data());
        }
        return vpass;
    });

    m_passes.insert(m_passes.begin(), m_prepass.get());
    m_passes.push_back(m_finpass.get());

    setRenderTargetSize(scene, rg);

    glslang::InitializeProcess();
    for(auto* p:m_passes) {
        if(!p->prepared()) {
            p->prepare(scene, *m_device, m_rendering_resources[0]);
        }
    }
    glslang::FinalizeProcess();

    VK_CHECK_RESULT_VOID_RE(m_upload_cmd.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit}));
    m_vertex_buf->recordUpload(m_upload_cmd);
    VK_CHECK_RESULT_VOID_RE(m_upload_cmd.end());
    {
    	vk::SubmitInfo sub_info;
		sub_info.setCommandBufferCount(1)
			.setPCommandBuffers(&m_upload_cmd);
		VK_CHECK_RESULT_VOID_RE(m_device->graphics_queue().handle.submit(1, &sub_info, {}));
        VK_CHECK_RESULT_VOID_RE(m_device->handle().waitIdle());
    }
    m_pass_loaded = true;
};


#include "CopyPass_impl.hpp"
#include "CustomShaderPass_impl.hpp"
#include "FinPass_impl.hpp"
#include "PrePass_impl.hpp"
#include "SceneToRenderGraph_impl.hpp"
