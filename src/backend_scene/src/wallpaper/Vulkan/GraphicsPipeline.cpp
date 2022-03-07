#include "GraphicsPipeline.hpp"
#include "Device.hpp"

#include "Utils/Logging.h"
#include "Utils/AutoDeletor.hpp"

using namespace wallpaper::vulkan;

namespace 
{

vk::ShaderModule CreateShaderModule(const vk::Device& device, ShaderSpv& spv) {
	auto& data = spv.spirv;
	vk::ShaderModuleCreateInfo createinfo;
	createinfo
		.setCodeSize(data.size() * sizeof(decltype(data.back())))
		.setPCode(data.data());
	return device.createShaderModule(createinfo).value;
}

}

GraphicsPipeline::GraphicsPipeline() {
    toDefault();
}
GraphicsPipeline::~GraphicsPipeline() {}

void GraphicsPipeline::toDefault() {
    m_view = vk::PipelineViewportStateCreateInfo{};
    m_view.setViewportCount(1)
        .setScissorCount(1);

    multisample = vk::PipelineMultisampleStateCreateInfo{};
    multisample.setRasterizationSamples(vk::SampleCountFlagBits::e1)
        .setMinSampleShading(1.0f)
        .setSampleShadingEnable(false)
        .setAlphaToOneEnable(false)
        .setAlphaToCoverageEnable(false);

    depth = vk::PipelineDepthStencilStateCreateInfo{};

    raster = vk::PipelineRasterizationStateCreateInfo{};
    raster.setCullMode(vk::CullModeFlagBits::eNone)
        .setFrontFace(vk::FrontFace::eCounterClockwise)
        .setDepthBiasEnable(false)
        .setDepthClampEnable(false)
        .setLineWidth(1.0f)
        .setPolygonMode(vk::PolygonMode::eFill);

    m_color_attachments.clear();
    m_color_attachments.emplace_back();
    m_color_attachments.front().setBlendEnable(false)
        .setColorWriteMask(
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA);
    m_color = vk::PipelineColorBlendStateCreateInfo{};
    m_color.setAttachmentCount(1)
        .setPAttachments(m_color_attachments.data())
        .setLogicOp(vk::LogicOp::eCopy)
        .setLogicOpEnable(false);

    m_dynamic_states = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };
}


ShaderSpv* GraphicsPipeline::getShaderSpv(vk::ShaderStageFlagBits stage) const {
    if(exists(m_stage_spv_map, stage))
        return m_stage_spv_map.at(stage).get();
    return nullptr;
}

GraphicsPipeline& GraphicsPipeline::setColorBlendStates(Span<vk::PipelineColorBlendAttachmentState> stats) {
    m_color_attachments = {stats.begin(), stats.end()};
    m_color.setAttachmentCount(m_color_attachments.size())
			.setPAttachments(m_color_attachments.data());
    return *this;
}

GraphicsPipeline& GraphicsPipeline::setLogicOp(bool enable, vk::LogicOp op) {
    m_color.setLogicOp(op)
			.setLogicOpEnable(enable);
    return *this;
}

GraphicsPipeline& GraphicsPipeline::setVertexInputState(const VertexInputState& in) {
    m_input_state = in;
    return *this;
}

GraphicsPipeline& GraphicsPipeline::setRenderPass(vk::RenderPass pass) {
    m_pass = pass;
    return *this;
}

GraphicsPipeline& GraphicsPipeline::addStage(Uni_ShaderSpv&& spv) {
    vk::ShaderStageFlagBits stage = spv->stage;
    m_stage_spv_map[stage] = std::move(spv);
    return *this;
}

GraphicsPipeline& GraphicsPipeline::addDescriptorSetInfo(Span<DescriptorSetInfo> info) {
    for(auto& i:info) {
        m_descriptor_set_infos.push_back(i);
    }
    return *this;
}

vk::Result GraphicsPipeline::create(const Device& device, PipelineParameters& pipeline) {
    vk::PipelineDynamicStateCreateInfo dynamic_info {};
    dynamic_info.setDynamicStateCount(m_dynamic_states.size())
        .setPDynamicStates(m_dynamic_states.data());

    for(auto& info:m_descriptor_set_infos){
        vk::DescriptorSetLayoutCreateInfo create_info;
        vk::DescriptorSetLayoutCreateFlags flags {};
        if(info.push_descriptor) flags |= vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR;
		create_info.setBindingCount(info.bindings.size())
			.setPBindings(info.bindings.data())
			.setFlags(flags);
		pipeline.descriptor_layouts.push_back(device.handle().createDescriptorSetLayout(create_info).value);
    }
   	{
		vk::PipelineLayoutCreateInfo create;
		create.setPSetLayouts(pipeline.descriptor_layouts.data());
		create.setSetLayoutCount(pipeline.descriptor_layouts.size());
		pipeline.layout = device.handle().createPipelineLayout(create).value;
	}

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    for(auto& item:m_stage_spv_map) {
        auto& spv = item.second;
        vk::PipelineShaderStageCreateInfo info;
        info.setStage(spv->stage)
            .setModule(CreateShaderModule(device.handle(), *spv))
            .setPName(spv->entry_point.c_str());
        shaderStages.push_back(info);
    }
    AUTO_DELETER(shadermodule, ([&shaderStages, &device]() {
        for(auto& sha:shaderStages) {
            device.handle().destroyShaderModule(sha.module);
        }
	}));

    vk::GraphicsPipelineCreateInfo create;
    create
        .setStageCount(shaderStages.size())
        .setPStages(shaderStages.data())
        .setPViewportState(&m_view)
        .setPDynamicState(&dynamic_info)
        .setPMultisampleState(&multisample)
        .setPDepthStencilState(&depth)
        .setPRasterizationState(&raster)
        .setRenderPass(m_pass)
        .setLayout(pipeline.layout)
        .setPColorBlendState(&m_color)
        .setPVertexInputState(&m_input_state.input)
        .setPInputAssemblyState(&m_input_state.input_assembly); 
    auto rv = device.handle().createGraphicsPipeline({}, create);
    pipeline.handle = rv.value;
    pipeline.pass = m_pass;
    return rv.result;
}