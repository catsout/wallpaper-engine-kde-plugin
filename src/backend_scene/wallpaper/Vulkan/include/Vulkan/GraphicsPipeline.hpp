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
	vk::Pipeline handle;
	vk::PipelineLayout layout;
  vk::RenderPass pass;
  std::vector<vk::DescriptorSetLayout> descriptor_layouts;
};

struct DescriptorSetInfo {
  bool push_descriptor {false};
  std::vector<vk::DescriptorSetLayoutBinding> bindings;
};

class Device;

class GraphicsPipeline {
public:
    GraphicsPipeline();
    ~GraphicsPipeline();

    void toDefault();
    bool create(const Device&, PipelineParameters&);

    vk::PipelineMultisampleStateCreateInfo   multisample;
    vk::PipelineRasterizationStateCreateInfo raster;
    vk::PipelineDepthStencilStateCreateInfo  depth;

    ShaderSpv* getShaderSpv(vk::ShaderStageFlagBits) const;
    const auto& pass() const { return m_pass; }

    GraphicsPipeline& setColorBlendStates(Span<vk::PipelineColorBlendAttachmentState>);
    GraphicsPipeline& setLogicOp(bool enable, vk::LogicOp);

    // required after default
    GraphicsPipeline& setRenderPass(vk::RenderPass);
    GraphicsPipeline& addDescriptorSetInfo(Span<DescriptorSetInfo>);
    GraphicsPipeline& addStage(Uni_ShaderSpv&&);
    GraphicsPipeline& addInputAttributeDescription(Span<vk::VertexInputAttributeDescription>);
    GraphicsPipeline& addInputBindingDescription(Span<vk::VertexInputBindingDescription>);
    GraphicsPipeline& setTopology(vk::PrimitiveTopology);
private:
    vk::RenderPass m_pass;

    vk::PipelineInputAssemblyStateCreateInfo m_input_assembly;
    std::vector<vk::VertexInputBindingDescription> m_input_bind_descriptions;
    std::vector<vk::VertexInputAttributeDescription> m_input_attr_descriptions;

    vk::PipelineViewportStateCreateInfo m_view;
    vk::PipelineColorBlendStateCreateInfo m_color;
    std::vector<vk::DynamicState> m_dynamic_states;
    std::vector<vk::PipelineColorBlendAttachmentState> m_color_attachments;
    std::vector<DescriptorSetInfo> m_descriptor_set_infos;
    Map<vk::ShaderStageFlagBits, Uni_ShaderSpv> m_stage_spv_map;
};

}
}