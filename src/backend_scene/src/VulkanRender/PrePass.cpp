#include "PrePass.hpp"
#include "Scene/Scene.h"
#include "Resource.hpp"

using namespace wallpaper::vulkan;

PrePass::PrePass(const Desc&) {}
PrePass::~PrePass() {}

namespace
{
TextureKey ToTexKey(wallpaper::SceneRenderTarget rt) {
    return TextureKey {
        .width  = rt.width,
        .height = rt.height,
        .usage  = {},
        .format = wallpaper::TextureFormat::RGBA8,
        .sample = rt.sample,
    };
}
} // namespace

void PrePass::prepare(Scene& scene, const Device& device, RenderingResources& rr) {
    {
        auto tex_name = std::string(m_desc.result);
        if (scene.renderTargets.count(tex_name) == 0) return;
        auto& rt = scene.renderTargets.at(tex_name);
        if (auto opt = device.tex_cache().Query(tex_name, ToTexKey(rt), ! rt.allowReuse);
            opt.has_value()) {
            m_desc.vk_result = opt.value();
        } else
            return;
    }
    {
        auto& sc           = scene.clearColor;
        m_desc.clear_value = VkClearValue { sc[0], sc[1], sc[2], 1.0f };
    }
    setPrepared();
}

void PrePass::execute(const Device& device, RenderingResources& rr) {
    auto&                   cmd = rr.command;
    VkImageSubresourceRange base_srang {
        .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = 0,
        .levelCount     = VK_REMAINING_ARRAY_LAYERS,
        .baseArrayLayer = 0,
        .layerCount     = VK_REMAINING_MIP_LEVELS,

    };
    {
        VkImageMemoryBarrier imb {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext            = nullptr,
            .srcAccessMask    = VK_ACCESS_MEMORY_READ_BIT,
            .dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .image            = m_desc.vk_result.handle,
            .subresourceRange = base_srang,
        };

        cmd.PipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_DEPENDENCY_BY_REGION_BIT,
                            imb);
    }
    cmd.ClearColorImage(m_desc.vk_result.handle,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        &m_desc.clear_value.color,
                        base_srang);
    VkImageMemoryBarrier imb {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext            = nullptr,
        .srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask    = VK_ACCESS_MEMORY_READ_BIT,
        .oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout        = m_desc.layout,
        .image            = m_desc.vk_result.handle,
        .subresourceRange = base_srang,
    };

    cmd.PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        VK_DEPENDENCY_BY_REGION_BIT,
                        imb);
}
void PrePass::destory(const Device& device, RenderingResources&) {
    setPrepared(false);
    clearReleaseTexs();
}
