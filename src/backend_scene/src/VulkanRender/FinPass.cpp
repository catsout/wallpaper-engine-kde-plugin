#include "FinPass.hpp"
#include "Vulkan/Shader.hpp"
#include "Resource.hpp"
#include "PassCommon.hpp"

using namespace wallpaper::vulkan;

constexpr std::string_view vert_code = R"(#version 320 es
layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 Texcoord;
layout(location = 0) out vec2 v_Texcoord;

void main()
{
	v_Texcoord = Texcoord;
	gl_Position = vec4(Position, 1.0);
}
)";

constexpr std::string_view frag_code = R"(#version 320 es
layout(location = 0) in vec2 v_Texcoord;
layout(location = 0) out vec4 out_FragColor;

// 0 is global ublock
layout(binding = 1) uniform sampler2D u_Texture;

void main()
{
	out_FragColor = texture(u_Texture, v_Texcoord);
}
)";

struct VertexInput {
    std::array<float, 3> pos;
    std::array<float, 2> color;
};

constexpr std::array vertex_input = {
    VertexInput { { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },
    VertexInput { { -1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    VertexInput { { 1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } },
    VertexInput { { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
};

FinPass::FinPass(const Desc&) {}
FinPass::~FinPass() {}
namespace
{
std::optional<vvk::RenderPass> CreateRenderPass(const vvk::Device& device, VkFormat format,
                                                VkImageLayout finalLayout) {
    VkAttachmentDescription attachment {
        .format         = format,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout    = finalLayout,
    };
    VkAttachmentReference attachment_ref {
        .attachment = 0,
        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass {
        .pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &attachment_ref,
    };

    VkRenderPassCreateInfo creatinfo {
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments    = &attachment,
        .subpassCount    = 1,
        .pSubpasses      = &subpass,
    };
    vvk::RenderPass pass;
    if (auto res = device.CreateRenderPass(creatinfo, pass); res == VK_SUCCESS) {
        return pass;
    } else {
        VVK_CHECK(res);
        return std::nullopt;
    }
}
} // namespace

void FinPass::setPresent(ImageParameters img) { m_desc.vk_present = img; }
void FinPass::setPresentLayout(VkImageLayout layout) { m_desc.present_layout = layout; }
void FinPass::setPresentFormat(VkFormat format) { m_desc.present_format = format; }
void FinPass::setPresentQueueIndex(uint32_t i) { m_desc.present_queue_index = i; }

void FinPass::prepare(Scene& scene, const Device& device, RenderingResources& rr) {
    {
        auto tex_name = std::string(m_desc.result);
        if (scene.renderTargets.count(tex_name) == 0) return;
        auto& rt = scene.renderTargets.at(tex_name);
        if (auto opt = device.tex_cache().Query(tex_name, ToTexKey(rt), ! rt.allowReuse);
            opt.has_value()) {
            m_desc.vk_result = opt.value();
        }
    }
    std::vector<Uni_ShaderSpv> spvs;
    {
        ShaderCompOpt opt;
        opt.client_ver             = glslang::EShTargetVulkan_1_1;
        opt.relaxed_errors_glsl    = true;
        opt.relaxed_rules_vulkan   = true;
        opt.suppress_warnings_glsl = true;

        std::array<ShaderCompUnit, 2> units;
        units[0] = ShaderCompUnit { .stage = EShLangVertex, .src = std::string(vert_code) };
        units[1] = ShaderCompUnit { .stage = EShLangFragment, .src = std::string(frag_code) };
        CompileAndLinkShaderUnits(units, opt, spvs);
    }

    VkVertexInputBindingDescription                bind_description;
    std::vector<VkVertexInputAttributeDescription> attr_descriptions;
    {
        bind_description.stride    = (sizeof(VertexInput));
        bind_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bind_description.binding   = (0);
        VkVertexInputAttributeDescription attr_pos, attr_color;
        attr_pos.binding    = (0);
        attr_pos.location   = (0);
        attr_pos.format     = VK_FORMAT_R32G32B32_SFLOAT;
        attr_pos.offset     = offsetof(VertexInput, pos);
        attr_color.binding  = (0);
        attr_color.location = (1);
        attr_color.format   = VK_FORMAT_R32G32_SFLOAT;
        attr_color.offset   = (offsetof(VertexInput, color));

        attr_descriptions.push_back(attr_pos);
        attr_descriptions.push_back(attr_color);

        {
            auto& buf = m_desc.vertex_buf;
            rr.vertex_buf->allocateSubRef(sizeof(decltype(vertex_input)), buf);
            rr.vertex_buf->writeToBuf(buf, { (uint8_t*)vertex_input.data(), buf.size });
        }
    }
    DescriptorSetInfo descriptor_info;
    {
        descriptor_info.push_descriptor = true;
        descriptor_info.bindings.resize(1);
        auto& binding           = descriptor_info.bindings.back();
        binding.binding         = (1);
        binding.descriptorCount = (1);
        binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    {
        auto opt = CreateRenderPass(device.handle(), m_desc.present_format, m_desc.present_layout);
        if (! opt.has_value()) return;
        auto pass = std::move(opt.value());

        descriptor_info.push_descriptor = true;
        GraphicsPipeline pipeline;
        pipeline.toDefault();
        pipeline.addDescriptorSetInfo(descriptor_info)
            .setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP)
            .addInputBindingDescription(bind_description)
            .addInputAttributeDescription(attr_descriptions);
        for (auto& spv : spvs) pipeline.addStage(std::move(spv));

        if (! pipeline.create(device, pass, m_desc.pipeline)) return;
    }
    /*
    if(m_desc.present_layout == vk::ImageLayout::ePresentSrcKHR || m_desc.present_layout ==
    vk::ImageLayout::eSharedPresentKHR) m_desc.render_layout = m_desc.present_layout; else
    m_desc.render_layout = vk::ImageLayout::eColorAttachmentOptimal;
    */

    m_desc.render_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    {
        auto& sc           = scene.clearColor;
        m_desc.clear_value = VkClearValue { { sc[0], sc[1], sc[2], 1.0f } };
    }
    setPrepared();
}

void FinPass::execute(const Device& device, RenderingResources& rr) {
    auto& cmd    = rr.command;
    auto& outext = m_desc.vk_present.extent;

    VkImageSubresourceRange base_srang {
        .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = 0,
        .levelCount     = VK_REMAINING_ARRAY_LAYERS,
        .baseArrayLayer = 0,
        .layerCount     = VK_REMAINING_MIP_LEVELS,

    };
    {
        m_desc.fb = {};
        VkFramebufferCreateInfo info {
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext           = nullptr,
            .renderPass      = *m_desc.pipeline.pass,
            .attachmentCount = 1,
            .pAttachments    = &m_desc.vk_present.view,
            .width           = m_desc.vk_present.extent.width,
            .height          = m_desc.vk_present.extent.height,
            .layers          = 1,
        };
        (void)device.handle().CreateFramebuffer(info, m_desc.fb);
    }
    {
        VkDescriptorImageInfo desc_img {
            .sampler     = m_desc.vk_result.sampler,
            .imageView   = m_desc.vk_result.view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
        VkWriteDescriptorSet wset {
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext           = nullptr,
            .dstSet          = {},
            .dstBinding      = 1,
            .descriptorCount = 1,
            .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo      = &desc_img,
        };
        cmd.PushDescriptorSetKHR(VK_PIPELINE_BIND_POINT_GRAPHICS, *m_desc.pipeline.layout, 0, wset);
    }

    // do queue family transfer operation
    if (m_desc.present_queue_index != device.graphics_queue().family_index) {
        VkImageMemoryBarrier imb {
            .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext               = nullptr,
            .srcAccessMask       = VK_ACCESS_MEMORY_READ_BIT,
            .dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout           = m_desc.present_layout,
            .newLayout           = m_desc.present_layout,
            .srcQueueFamilyIndex = m_desc.present_queue_index,
            .dstQueueFamilyIndex = device.graphics_queue().family_index,
            .image               = m_desc.vk_present.handle,
            .subresourceRange    = base_srang,
        };

        cmd.PipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                            VK_DEPENDENCY_BY_REGION_BIT,
                            imb);
    }
    VkRenderPassBeginInfo pass_begin_info {
        .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext       = nullptr,
        .renderPass  = *m_desc.pipeline.pass,
        .framebuffer = *m_desc.fb,
        .renderArea =
            VkRect2D {
                .offset = { 0, 0 },
                .extent = { outext.width, outext.height },
            },
        .clearValueCount = 1,
        .pClearValues    = &m_desc.clear_value,
    };
    cmd.BeginRenderPass(pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    cmd.BindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, *m_desc.pipeline.handle);
    VkViewport viewport {
        .x        = 0,
        .y        = (float)outext.height,
        .width    = (float)outext.width,
        .height   = -(float)outext.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    VkRect2D scissor { { 0, 0 }, { outext.width, outext.height } };
    cmd.SetViewport(0, viewport);
    cmd.SetScissor(0, scissor);

    cmd.BindVertexBuffers(
        0, 1, std::array { rr.vertex_buf->gpuBuf() }.data(), &m_desc.vertex_buf.offset);
    cmd.Draw(4, 1, 0, 0);
    cmd.EndRenderPass();

    // do queue family transfer operation
    if (m_desc.present_queue_index != device.graphics_queue().family_index) {
        VkImageMemoryBarrier imb {
            .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext               = nullptr,
            .srcAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask       = VK_ACCESS_MEMORY_READ_BIT,
            .oldLayout           = m_desc.present_layout,
            .newLayout           = m_desc.present_layout,
            .srcQueueFamilyIndex = device.graphics_queue().family_index,
            .dstQueueFamilyIndex = m_desc.present_queue_index,
            .image               = m_desc.vk_present.handle,
            .subresourceRange    = base_srang,
        };

        cmd.PipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VK_DEPENDENCY_BY_REGION_BIT,
                            imb);
    }
}
void FinPass::destory(const Device& device, RenderingResources& rr) {
    setPrepared(false);
    clearReleaseTexs();
    rr.vertex_buf->unallocateSubRef(m_desc.vertex_buf);
}
