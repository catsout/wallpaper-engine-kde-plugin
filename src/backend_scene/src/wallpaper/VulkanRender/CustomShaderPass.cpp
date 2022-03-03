#include "CustomShaderPass.hpp"
#include "Scene/Scene.h"

#include "SpecTexs.h"
#include "Vulkan/VertexInputState.hpp"
#include "Vulkan/Shader.hpp"
#include "Utils/Logging.h"

#include <cassert>

using namespace wallpaper::vulkan;

CustomShaderPass::CustomShaderPass(const Desc& desc):m_desc(desc) {};
CustomShaderPass::~CustomShaderPass() {

}

struct PipelineParameters {
	vk::Pipeline handle;
	vk::PipelineLayout layout;
};

static vk::ShaderModule CreateShaderModule(const vk::Device& device, ShaderSpv& spv) {
	auto& data = spv.spirv;
	vk::ShaderModuleCreateInfo createinfo;
	createinfo
		.setCodeSize(data.size() * sizeof(uint32_t))
		.setPCode(data.data());
	return device.createShaderModule(createinfo).value;
}

static vk::ShaderStageFlagBits ToVkType(EShLanguage esh) {
	switch (esh)
	{
	case EShLangVertex:
		return vk::ShaderStageFlagBits::eVertex;
	case EShLangFragment:
		return vk::ShaderStageFlagBits::eFragment;
	default:
		return vk::ShaderStageFlagBits::eVertex;
	}
}


vk::RenderPass CreateRenderPass(const vk::Device &device, vk::Format format)
{
	vk::AttachmentDescription attachment;
	attachment
		.setFormat(format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	vk::AttachmentReference attachment_ref;
	attachment_ref
		.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::SubpassDescription subpass;
	subpass
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&attachment_ref);

	vk::RenderPassCreateInfo creatinfo;
	creatinfo
		.setAttachmentCount(1)
		.setPAttachments(&attachment)
		.setSubpassCount(1)
		.setPSubpasses(&subpass);
	return device.createRenderPass(creatinfo).value;
}

vk::ResultValue<PipelineParameters> CreatePipline(const vk::Device &device, vk::RenderPass &pass,
												Span<Uni_ShaderSpv> spvs,
												const VertexInputState& input_state,
												Span<vk::DescriptorSetLayout> descriptor_layouts)
{
	PipelineParameters pipeline;
	glslang::InitializeProcess();
	glslang::FinalizeProcess();

	{
		vk::PipelineLayoutCreateInfo create;
		create.setPSetLayouts(descriptor_layouts.data());
		create.setSetLayoutCount(descriptor_layouts.size());
		pipeline.layout = device.createPipelineLayout(create).value;
	}
	{
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		for(auto& spv:spvs) {
			vk::PipelineShaderStageCreateInfo info;
			info.setStage(ToVkType(spv->stage))
				.setModule(CreateShaderModule(device, *spv))
				.setPName("main");
			shaderStages.push_back(info);
		}

		vk::PipelineViewportStateCreateInfo view;
		view.setViewportCount(1)
			.setScissorCount(1);

		vk::PipelineMultisampleStateCreateInfo multisample;
		multisample.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setMinSampleShading(1.0f)
			.setSampleShadingEnable(false)
			.setAlphaToOneEnable(false)
			.setAlphaToCoverageEnable(false);

		vk::PipelineDepthStencilStateCreateInfo depth;

		vk::PipelineDynamicStateCreateInfo dynamic;
		{
			std::array<vk::DynamicState, 2> dynamicstats{
				vk::DynamicState::eViewport,
				vk::DynamicState::eScissor};
			dynamic.setDynamicStateCount(dynamicstats.size())
				.setPDynamicStates(dynamicstats.data());
		}

		vk::PipelineRasterizationStateCreateInfo raster;
		raster.setCullMode(vk::CullModeFlagBits::eBack)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setDepthBiasEnable(false)
			.setDepthClampEnable(false)
			.setLineWidth(1.0f)
			.setPolygonMode(vk::PolygonMode::eFill);

		vk::PipelineColorBlendStateCreateInfo color;
		vk::PipelineColorBlendAttachmentState colorattach;
		colorattach.setBlendEnable(false)
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA);
		color.setAttachmentCount(1)
			.setPAttachments(&colorattach)
			.setLogicOp(vk::LogicOp::eCopy)
			.setLogicOpEnable(false);

		vk::GraphicsPipelineCreateInfo create;
		create
			.setStageCount(shaderStages.size())
			.setPStages(shaderStages.data())
			.setPViewportState(&view)
			.setPDynamicState(&dynamic)
			.setPMultisampleState(&multisample)
			.setPDepthStencilState(&depth)
			.setPRasterizationState(&raster)
			.setRenderPass(pass)
			.setLayout(pipeline.layout)
			.setPColorBlendState(&color)
			.setPVertexInputState(&input_state.input)
			.setPInputAssemblyState(&input_state.input_assembly);
		auto value = device.createGraphicsPipeline({}, create);
		pipeline.handle = value.value;
		vk::ResultValue<PipelineParameters> result(value.result, pipeline);
		return result;
	}
}

static TextureKey ToTexKey(wallpaper::SceneRenderTarget rt) {
	return TextureKey {
		.width = rt.width,
		.height = rt.height,
		.usage = {},
		.format = wallpaper::TextureFormat::RGBA8,
		.sample = rt.sample
	};
}

void CustomShaderPass::prepare(Scene& scene, const Device& device, RenderingResources& rr) {
    m_desc.vk_textures.resize(m_desc.textures.size());
    for(int i=0;i < m_desc.textures.size();i++) {
        auto& tex_name = m_desc.textures[i];
		if(tex_name.empty()) continue;

		vk::ResultValue<ImageSlots> rv {vk::Result::eSuccess, {}};
        if(IsSpecTex(tex_name)) {
			if(scene.renderTargets.count(tex_name) == 0) continue;
			auto& rt = scene.renderTargets.at(tex_name);
			auto rv_paras = device.tex_cache().Query(tex_name, ToTexKey(rt));
			rv.result = rv_paras.result;
			rv.value = ImageSlots{{rv_paras.value}, 0};
		} else {
			auto image = scene.imageParser->Parse(tex_name);
			rv = device.tex_cache().CreateTex(*image);
		}
		if(rv.result == vk::Result::eSuccess)
			m_desc.vk_textures[i] = rv.value;
    }
	{
		auto& tex_name = m_desc.output;
		assert(IsSpecTex(tex_name));
		assert(scene.renderTargets.count(tex_name) > 0);
		auto& rt = scene.renderTargets.at(tex_name);
		auto rv = device.tex_cache().Query(tex_name, ToTexKey(rt));
		if(rv.result == vk::Result::eSuccess)
			m_desc.vk_output = rv.value;
	}

	SceneMesh& mesh = *(m_desc.node->Mesh());

    std::vector<Uni_ShaderSpv> spvs;
	std::vector<vk::DescriptorSetLayout> descriptro_layouts;
	ShaderReflected ref;
    {
        SceneShader& shader = *(mesh.Material()->customShader.shader);
        ShaderCompOpt opt;
        opt.client_ver = glslang::EShTargetVulkan_1_1;
        opt.auto_map_bindings = true;
        opt.auto_map_locations = true;
        opt.relaxed_errors_glsl = true;
        opt.relaxed_rules_vulkan = true;
        opt.suppress_warnings_glsl = true;

        std::vector<ShaderCompUnit> units;
        if(!shader.vertexCode.empty()) {
            units.push_back(ShaderCompUnit{.stage=EShLangVertex, .src=shader.vertexCode});
        }
        if(!shader.fragmentCode.empty()) {
            units.push_back(ShaderCompUnit{.stage=EShLangFragment, .src=shader.fragmentCode});
        }
        CompileAndLinkShaderUnits(units, opt, spvs, ref);
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		bindings.resize(ref.binding_map.size());
		LOG_INFO("----shader------");
		LOG_INFO("--inputs:");
		for(auto& i:ref.input_location_map) {
			LOG_INFO("%d %s", i.second, i.first.c_str());
		}
		LOG_INFO("--bindings:");
		std::transform(ref.binding_map.begin(), ref.binding_map.end(), bindings.begin(), [](auto& item) {
			LOG_INFO(item.first.c_str());
			return item.second;
		});
		vk::DescriptorSetLayoutCreateInfo info;
		info.setBindingCount(bindings.size())
			.setPBindings(bindings.data());
		descriptro_layouts.push_back(device.handle().createDescriptorSetLayout(info).value);
    }

	VertexInputState input_state;
	{
		for(int i=0;i<mesh.VertexCount();i++) {
			const auto& vertex = mesh.GetVertexArray(i);
			auto attrs = vertex.GetAttrOffset();

			vk::VertexInputBindingDescription bind_desc;
			bind_desc.setStride(vertex.OneSizeOf()).setInputRate(vk::VertexInputRate::eVertex).setBinding(i);
			input_state.bind_descriptions.push_back(bind_desc);

			for(auto& attr:attrs) {
				vk::VertexInputAttributeDescription attr_desc;
				if(!exists(ref.input_location_map, attr.attr.name)) continue;
				attr_desc.setBinding(i)
					.setLocation(ref.input_location_map.at(attr.attr.name))
					.setFormat(vk::Format::eR32G32Sfloat)
					.setOffset(attr.offset);
				input_state.attr_descriptions.push_back(attr_desc);
			}
			input_state.input.setPVertexBindingDescriptions(input_state.bind_descriptions.data())
				.setPVertexAttributeDescriptions(input_state.attr_descriptions.data())
				.setVertexBindingDescriptionCount(input_state.bind_descriptions.size())
				.setVertexAttributeDescriptionCount(input_state.attr_descriptions.size());

			input_state.input_assembly.setTopology(vk::PrimitiveTopology::eTriangleStrip)
				.setPrimitiveRestartEnable(false);
		}	
	}
	auto pass = CreateRenderPass(device.handle(), vk::Format::eR8G8B8A8Unorm);
	CreatePipline(device.handle(), pass, spvs, input_state, descriptro_layouts);
    setPrepared();
}

void CustomShaderPass::execute(const Device& device, RenderingResources& rr) {

}

void CustomShaderPass::destory(const Device& device) {

}