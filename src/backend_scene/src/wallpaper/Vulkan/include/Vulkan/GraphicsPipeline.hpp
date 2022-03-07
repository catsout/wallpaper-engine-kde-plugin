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

struct VertexInputState {
  vk::PipelineInputAssemblyStateCreateInfo input_assembly;
  vk::PipelineVertexInputStateCreateInfo input;
  std::vector<vk::VertexInputBindingDescription> bind_descriptions;
  std::vector<vk::VertexInputAttributeDescription> attr_descriptions;
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
    vk::Result create(const Device&, PipelineParameters&);

    vk::PipelineMultisampleStateCreateInfo   multisample;
    vk::PipelineRasterizationStateCreateInfo raster;
    vk::PipelineDepthStencilStateCreateInfo  depth;

    ShaderSpv* getShaderSpv(vk::ShaderStageFlagBits) const;
    const auto& pass() const { return m_pass; }

    GraphicsPipeline& setColorBlendStates(Span<vk::PipelineColorBlendAttachmentState>);
    GraphicsPipeline& setLogicOp(bool enable, vk::LogicOp);

    // required after default
    GraphicsPipeline& setVertexInputState(const VertexInputState&);
    GraphicsPipeline& setRenderPass(vk::RenderPass);
    GraphicsPipeline& addDescriptorSetInfo(Span<DescriptorSetInfo>);
    GraphicsPipeline& addStage(Uni_ShaderSpv&&);
private:
    vk::RenderPass m_pass;

    VertexInputState m_input_state;

    vk::PipelineViewportStateCreateInfo m_view;
    vk::PipelineColorBlendStateCreateInfo m_color;
    std::vector<vk::DynamicState> m_dynamic_states;
    std::vector<vk::PipelineColorBlendAttachmentState> m_color_attachments;
    std::vector<DescriptorSetInfo> m_descriptor_set_infos;
    Map<vk::ShaderStageFlagBits, Uni_ShaderSpv> m_stage_spv_map;
};

}
}