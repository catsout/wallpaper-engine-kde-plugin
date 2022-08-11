#include "GraphicsPipeline.hpp"
#include "Device.hpp"

#include "TextureCache.hpp"
#include "Utils/Logging.h"
#include "Utils/AutoDeletor.hpp"
#include "vvk/vulkan_wrapper.hpp"
#include <cstdint>

using namespace wallpaper::vulkan;

namespace
{

inline VkShaderStageFlagBits ToVkType(wallpaper::ShaderType stage) {
    using namespace wallpaper;
    switch (stage) {
    case ShaderType::VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
    case ShaderType::FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
    case ShaderType::GEOMETRY: return VK_SHADER_STAGE_GEOMETRY_BIT;
    default: assert(false); return VK_SHADER_STAGE_VERTEX_BIT;
    }
}

inline std::optional<vvk::ShaderModule> CreateShaderModule(const vvk::Device& device,
                                                           ShaderSpv&         spv) {
    auto&                    data = spv.spirv;
    VkShaderModuleCreateInfo ci {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext    = nullptr,
        .codeSize = data.size() * sizeof(decltype(data.back())),
        .pCode    = data.data(),
    };
    vvk::ShaderModule sm;
    VVK_CHECK_ACT(return std::nullopt, device.CreateShaderModule(ci, sm));
    return sm;
}

} // namespace

GraphicsPipeline::GraphicsPipeline() { toDefault(); }
GraphicsPipeline::~GraphicsPipeline() {}

void GraphicsPipeline::toDefault() {
    m_view = VkPipelineViewportStateCreateInfo {
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext         = nullptr,
        .viewportCount = 1,
        .scissorCount  = 1

    };
    multisample = VkPipelineMultisampleStateCreateInfo {
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext                 = nullptr,
        .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable   = false,
        .minSampleShading      = 1.0f,
        .alphaToCoverageEnable = false,
        .alphaToOneEnable      = false,
    };

    depth = VkPipelineDepthStencilStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, .pNext = nullptr
    };

    raster = VkPipelineRasterizationStateCreateInfo {
        .sType            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext            = nullptr,
        .depthClampEnable = false,
        .polygonMode      = VK_POLYGON_MODE_FILL,
        .cullMode         = VK_CULL_MODE_NONE,
        .frontFace        = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable  = false,
        .lineWidth        = 1.0f,
    };
    m_color_attachments.clear();
    m_color_attachments.push_back(VkPipelineColorBlendAttachmentState {
        .blendEnable    = false,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT });
    m_color = VkPipelineColorBlendStateCreateInfo {
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext           = nullptr,
        .logicOpEnable   = false,
        .logicOp         = VK_LOGIC_OP_COPY,
        .attachmentCount = (uint32_t)m_color_attachments.size(),
        .pAttachments    = m_color_attachments.data(),
    };
    m_dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    m_input_assembly = VkPipelineInputAssemblyStateCreateInfo {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext                  = nullptr,
        .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        .primitiveRestartEnable = false
    };
}

ShaderSpv* GraphicsPipeline::getShaderSpv(VkShaderStageFlagBits stage) const {
    if (exists(m_stage_spv_map, stage)) return m_stage_spv_map.at(stage).get();
    return nullptr;
}

GraphicsPipeline&
GraphicsPipeline::setColorBlendStates(std::span<const VkPipelineColorBlendAttachmentState> stats) {
    m_color_attachments     = { stats.begin(), stats.end() };
    m_color.attachmentCount = (u32)m_color_attachments.size();
    m_color.pAttachments    = m_color_attachments.data();
    return *this;
}

GraphicsPipeline& GraphicsPipeline::setLogicOp(bool enable, VkLogicOp op) {
    m_color.logicOp       = op;
    m_color.logicOpEnable = enable;
    return *this;
}

GraphicsPipeline& GraphicsPipeline::setRenderPass(vvk::RenderPass pass) {
    m_pass = std::move(pass);
    return *this;
}

GraphicsPipeline& GraphicsPipeline::addStage(Uni_ShaderSpv&& spv) {
    VkShaderStageFlagBits stage = ::ToVkType(spv->stage);
    m_stage_spv_map[stage]      = std::move(spv);
    return *this;
}

GraphicsPipeline& GraphicsPipeline::addDescriptorSetInfo(std::span<const DescriptorSetInfo> info) {
    for (auto& i : info) {
        m_descriptor_set_infos.push_back(i);
    }
    return *this;
}

GraphicsPipeline& GraphicsPipeline::addInputAttributeDescription(
    std::span<const VkVertexInputAttributeDescription> attrs) {
    for (auto& a : attrs) {
        m_input_attr_descriptions.push_back(a);
    }
    return *this;
}
GraphicsPipeline& GraphicsPipeline::addInputBindingDescription(
    std::span<const VkVertexInputBindingDescription> binds) {
    for (auto& b : binds) {
        m_input_bind_descriptions.push_back(b);
    }
    return *this;
}
GraphicsPipeline& GraphicsPipeline::setTopology(VkPrimitiveTopology topology) {
    m_input_assembly.topology = topology;
    return *this;
}

bool GraphicsPipeline::create(const Device& device, vvk::RenderPass& pass,
                              PipelineParameters& pipeline) {
    VkPipelineDynamicStateCreateInfo dynamic_info {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext             = nullptr,
        .dynamicStateCount = (uint32_t)m_dynamic_states.size(),
        .pDynamicStates    = m_dynamic_states.data()
    };
    for (auto& info : m_descriptor_set_infos) {
        VkDescriptorSetLayoutCreateInfo create_info {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .pNext = nullptr
        };
        VkDescriptorSetLayoutCreateFlags flags {};
        if (info.push_descriptor) flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;

        create_info.bindingCount = (u32)info.bindings.size();
        create_info.pBindings    = info.bindings.data();
        create_info.flags        = flags;
        vvk::DescriptorSetLayout layout;
        VVK_CHECK(device.handle().CreateDescriptorSetLayout(create_info, layout));
        pipeline.descriptor_layouts.emplace_back(std::move(layout));
    }
    {
        std::vector<VkDescriptorSetLayout> layouts =
            vvk::ToVector<vvk::DescriptorSetLayout>(pipeline.descriptor_layouts);

        VkPipelineLayoutCreateInfo ci {
            .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext          = nullptr,
            .setLayoutCount = (uint32_t)layouts.size(),
            .pSetLayouts    = layouts.data(),
        };
        VVK_CHECK(device.handle().CreatePipelineLayout(ci, pipeline.layout));
    }

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    std::vector<vvk::ShaderModule>               shader_modules;
    for (auto& item : m_stage_spv_map) {
        auto&                           spv = item.second;
        VkPipelineShaderStageCreateInfo info {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .stage = ::ToVkType(spv->stage),
            .pName = spv->entry_point.c_str()
        };
        if (auto opt = CreateShaderModule(device.handle(), *spv); opt.has_value()) {
            shader_modules.emplace_back(std::move(opt.value()));
            info.module = *shader_modules.back();
        }

        shaderStages.push_back(info);
    }
    /*
    AUTO_DELETER(shadermodule, ([&shaderStages, &device]() {
                     for (auto& sha : shaderStages) {
                         device.handle().destroyShaderModule(sha.module);
                     }
                 }));
    */

    VkPipelineVertexInputStateCreateInfo input {
        .sType                         = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext                         = nullptr,
        .vertexBindingDescriptionCount = (uint32_t)m_input_bind_descriptions.size(),
        .pVertexBindingDescriptions    = m_input_bind_descriptions.data(),
        .vertexAttributeDescriptionCount = (uint32_t)m_input_attr_descriptions.size(),
        .pVertexAttributeDescriptions    = m_input_attr_descriptions.data()
    };

    VkGraphicsPipelineCreateInfo create {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = nullptr,
        .stageCount          = (uint32_t)shaderStages.size(),
        .pStages             = shaderStages.data(),
        .pVertexInputState   = &input,
        .pInputAssemblyState = &m_input_assembly,
        .pViewportState      = &m_view,
        .pRasterizationState = &raster,
        .pMultisampleState   = &multisample,
        .pDepthStencilState  = &depth,
        .pColorBlendState    = &m_color,
        .pDynamicState       = &dynamic_info,
        .layout              = *pipeline.layout,
        .renderPass          = *pass,
    };
    VVK_CHECK_BOOL_RE(device.handle().CreateGraphicsPipeline(create, pipeline.handle));
    pipeline.pass = std::move(pass);
    return true;
}
