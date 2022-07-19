#pragma once
#include "Instance.hpp"
#include "Utils/span.hpp"
#include "Utils/MapSet.hpp"
#include "Spv.hpp"

namespace wallpaper
{
namespace vulkan
{

struct PipelineParameters {
    vvk::Pipeline       handle;
    vvk::PipelineLayout layout;
    vvk::RenderPass     pass;

    std::vector<vvk::DescriptorSetLayout> descriptor_layouts;
};

struct DescriptorSetInfo {
    bool push_descriptor { false };

    std::vector<VkDescriptorSetLayoutBinding> bindings;
};

class Device;

class GraphicsPipeline : NoCopy, NoMove {
public:
    GraphicsPipeline();
    ~GraphicsPipeline();

    void toDefault();
    bool create(const Device&, vvk::RenderPass&, PipelineParameters&);

    VkPipelineMultisampleStateCreateInfo   multisample {};
    VkPipelineRasterizationStateCreateInfo raster {};
    VkPipelineDepthStencilStateCreateInfo  depth {};

    ShaderSpv*  getShaderSpv(VkShaderStageFlagBits) const;
    const auto& pass() const { return m_pass; }

    GraphicsPipeline& setColorBlendStates(Span<const VkPipelineColorBlendAttachmentState>);
    GraphicsPipeline& setLogicOp(bool enable, VkLogicOp);

    // required after default
    GraphicsPipeline& setRenderPass(vvk::RenderPass);
    GraphicsPipeline& addDescriptorSetInfo(Span<const DescriptorSetInfo>);
    GraphicsPipeline& addStage(Uni_ShaderSpv&&);
    GraphicsPipeline& addInputAttributeDescription(Span<const VkVertexInputAttributeDescription>);
    GraphicsPipeline& addInputBindingDescription(Span<const VkVertexInputBindingDescription>);
    GraphicsPipeline& setTopology(VkPrimitiveTopology);

private:
    vvk::RenderPass m_pass;

    VkPipelineInputAssemblyStateCreateInfo         m_input_assembly {};
    std::vector<VkVertexInputBindingDescription>   m_input_bind_descriptions;
    std::vector<VkVertexInputAttributeDescription> m_input_attr_descriptions;

    VkPipelineViewportStateCreateInfo                m_view;
    VkPipelineColorBlendStateCreateInfo              m_color;
    std::vector<VkDynamicState>                      m_dynamic_states;
    std::vector<VkPipelineColorBlendAttachmentState> m_color_attachments;
    std::vector<DescriptorSetInfo>                   m_descriptor_set_infos;
    Map<VkShaderStageFlagBits, Uni_ShaderSpv>        m_stage_spv_map;
};

} // namespace vulkan
} // namespace wallpaper
