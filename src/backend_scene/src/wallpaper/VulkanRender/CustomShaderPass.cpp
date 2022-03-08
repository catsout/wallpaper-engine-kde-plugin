#include "CustomShaderPass.hpp"
#include "Scene/Scene.h"
#include "Scene/SceneShader.h"

#include "SpecTexs.h"
#include "Vulkan/Shader.hpp"
#include "Utils/Logging.h"
#include "Utils/AutoDeletor.hpp"
#include "Resource.hpp"
#include "PassCommon.hpp"

#include <cassert>

using namespace wallpaper::vulkan;


CustomShaderPass::CustomShaderPass(const Desc& desc):m_desc(desc) {};
CustomShaderPass::~CustomShaderPass() {}

vk::RenderPass CreateRenderPass(const vk::Device &device, vk::Format format, vk::AttachmentLoadOp loadOp)
{
	vk::AttachmentDescription attachment;
	attachment
		.setFormat(format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(loadOp)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	if(loadOp == vk::AttachmentLoadOp::eLoad) {
		attachment.initialLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	}

	vk::AttachmentReference attachment_ref;
	attachment_ref
		.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::SubpassDescription subpass;
	subpass
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&attachment_ref);

	vk::SubpassDependency dependency;
	dependency
		.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eFragmentShader)
		.setSrcAccessMask({})
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	vk::RenderPassCreateInfo creatinfo;
	creatinfo
		.setAttachmentCount(1)
		.setPAttachments(&attachment)
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(1)
		.setPDependencies(&dependency);
	return device.createRenderPass(creatinfo).value;
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

static void UpdateUniform(StagingBuffer* buf, 
	const StagingBufferRef& bufref, 
	const ShaderReflected::Block& block, 
	std::string_view name,
	const wallpaper::ShaderValue& value) {
	using namespace wallpaper;
	Span<uint8_t> value_u8 {(uint8_t*)value.data(), value.size()*sizeof(ShaderValue::value_type)};
	auto uni = block.member_map.find(name);
	size_t offset = uni->second.offset;
	size_t type_size = Sizeof(uni->second.type);
	if(type_size != value_u8.size()) {
		;//to do
	}
	buf->writeToBuf(bufref, value_u8, offset);
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
		VK_CHECK_RESULT(rv.result);
		if(rv.result == vk::Result::eSuccess)
			m_desc.vk_textures[i] = rv.value;
    }
	{
		auto& tex_name = m_desc.output;
		assert(IsSpecTex(tex_name));
		assert(scene.renderTargets.count(tex_name) > 0);
		auto& rt = scene.renderTargets.at(tex_name);
		auto rv = device.tex_cache().Query(tex_name, ToTexKey(rt));
		VK_CHECK_RESULT_VOID_RE(rv.result);
		if(rv.result == vk::Result::eSuccess)
			m_desc.vk_output = rv.value;
	}

	SceneMesh& mesh = *(m_desc.node->Mesh());

    std::vector<Uni_ShaderSpv> spvs;
	DescriptorSetInfo descriptor_info;
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
        CompileAndLinkShaderUnits(units, opt, spvs, &ref);

		auto& bindings = descriptor_info.bindings;
		bindings.resize(ref.binding_map.size());
		LOG_INFO("----shader------");
		LOG_INFO("%s", shader.name.c_str());
		LOG_INFO("--inputs:");
		for(auto& i:ref.input_location_map) {
			LOG_INFO("%d %s", i.second, i.first.c_str());
		}
		LOG_INFO("--bindings:");
		std::transform(ref.binding_map.begin(), ref.binding_map.end(), bindings.begin(), [](auto& item) {
			LOG_INFO("%d %s", item.second.binding, item.first.c_str());
			return item.second;
		});

		for(int i=0;i<m_desc.vk_textures.size();i++) {
			int binding {-1};
			if(exists(ref.binding_map, WE_GLTEX_NAMES[i]))
				binding = ref.binding_map.at(WE_GLTEX_NAMES[i]).binding;
			m_desc.vk_tex_binding.push_back(binding);
		}
    }

	std::vector<vk::VertexInputBindingDescription> bind_descriptions;
	std::vector<vk::VertexInputAttributeDescription> attr_descriptions;
	{
		m_desc.vertex_bufs.resize(mesh.VertexCount());
		for(int i=0;i<mesh.VertexCount();i++) {
			const auto& vertex = mesh.GetVertexArray(i);
			auto attrs_map = vertex.GetAttrOffsetMap();

			vk::VertexInputBindingDescription bind_desc;
			bind_desc.setStride(vertex.OneSizeOf()).setInputRate(vk::VertexInputRate::eVertex).setBinding(i);
			bind_descriptions.push_back(bind_desc);
			for(auto& item:ref.input_location_map) {
				auto& name = item.first;
				auto& input = item.second;
				uint offset = exists(attrs_map, name) ? attrs_map[name].offset : 0;

				vk::VertexInputAttributeDescription attr_desc;
				attr_desc.setBinding(i)
					.setLocation(input.location)
					.setFormat(input.format)
					.setOffset(offset);
				attr_descriptions.push_back(attr_desc);
			}
			{
				auto& buf = m_desc.vertex_bufs[i];
				rr.vertex_buf->allocateSubRef(vertex.DataSizeOf(), buf);
				rr.vertex_buf->writeToBuf(buf, {(uint8_t*)vertex.Data(), buf.size});
			}
		}
	}
	{
		vk::PipelineColorBlendAttachmentState color_blend;
		{
			vk::ColorComponentFlags colorMask =
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB;
			bool alpha = !(m_desc.node->Camera().empty() || m_desc.node->Camera().compare(0, 6, "global") == 0);
			if(alpha) colorMask |= vk::ColorComponentFlagBits::eA;
			color_blend.setColorWriteMask(colorMask);
			SetBlend(mesh.Material()->blenmode, color_blend);
			m_desc.blending = color_blend.blendEnable;
		}
		vk::AttachmentLoadOp loadOp {vk::AttachmentLoadOp::eDontCare};
		if(m_desc.blending) loadOp = vk::AttachmentLoadOp::eLoad;
		auto pass = CreateRenderPass(device.handle(), vk::Format::eR8G8B8A8Unorm, loadOp);
		descriptor_info.push_descriptor = true;
		GraphicsPipeline pipeline;
		pipeline.toDefault();
		pipeline.setRenderPass(pass)
			.addDescriptorSetInfo(descriptor_info)
			.setColorBlendStates(color_blend)
			.setTopology(vk::PrimitiveTopology::eTriangleStrip)
			.addInputBindingDescription(bind_descriptions)
			.addInputAttributeDescription(attr_descriptions);
		for(auto& spv:spvs) pipeline.addStage(std::move(spv));

		pipeline.create(device, m_desc.pipeline);
	}

	{
		vk::FramebufferCreateInfo info;
		info.setRenderPass(m_desc.pipeline.pass)
			.setAttachmentCount(1)
			.setPAttachments(&m_desc.vk_output.view)
			.setWidth(m_desc.vk_output.extent.width)
			.setHeight(m_desc.vk_output.extent.height)
			.setLayers(1);
	 	VK_CHECK_RESULT_VOID_RE(device.handle().createFramebuffer(&info, nullptr, &m_desc.fb));
	}

	if(!ref.blocks.empty()){
		auto& block = ref.blocks.front();
		rr.ubo_buf->allocateSubRef(block.size, m_desc.ubo_buf, device.limits().minUniformBufferOffsetAlignment);
	}

	if(!ref.blocks.empty()) {
		auto block = ref.blocks.front();
		auto* shader_updater = scene.shaderValueUpdater.get();
		auto* buf = rr.ubo_buf;
		auto* bufref = &m_desc.ubo_buf;
		auto* node = m_desc.node;
		m_desc.update_uniform_op = [block, shader_updater, buf, bufref, node]() {
			auto existsOp = [&block](std::string_view name) {
				return exists(block.member_map, name);
			};
			auto updateOp = [&block, buf, bufref](std::string_view name, wallpaper::ShaderValue value) {
				UpdateUniform(buf, *bufref, block, name, value);
			};
			shader_updater->UpdateUniforms(node, existsOp, updateOp);
		};
		{
			auto& default_values = mesh.Material()->customShader.shader->default_uniforms;
			auto& const_values = mesh.Material()->customShader.constValues;
			std::array values_array = {&default_values, &const_values};
			for(auto& values:values_array) {
				for(auto& v:*values) {
					if(exists(block.member_map, v.first)) {
						UpdateUniform(buf, *bufref, block, v.first, v.second);
					} 
				}
			}
		}
		m_desc.update_uniform_op();
	} else {
		m_desc.update_uniform_op = [](){};
	}
	{
		auto& sc = scene.clearColor;
		m_desc.clear_value = vk::ClearValue(std::array {sc[0], sc[1], sc[2], 1.0f});
	}
    setPrepared();
}

void CustomShaderPass::execute(const Device& device, RenderingResources& rr) {
	m_desc.update_uniform_op();

	auto& cmd = rr.command;
	auto& outext = m_desc.vk_output.extent;
	vk::ImageSubresourceRange base_range;
	base_range.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseArrayLayer(0)
		.setBaseMipLevel(0)
		.setLayerCount(VK_REMAINING_ARRAY_LAYERS)
		.setLevelCount(VK_REMAINING_MIP_LEVELS);

	for(int i=0;i<m_desc.vk_textures.size();i++){
		auto& slot = m_desc.vk_textures[i];
		int binding = m_desc.vk_tex_binding[i];
		if(binding < 0) continue;
		if(slot.slots.empty()) continue;
		auto& img = slot.slots[slot.active];
		vk::DescriptorImageInfo desc_img {img.sampler, img.view, vk::ImageLayout::eShaderReadOnlyOptimal};
		vk::WriteDescriptorSet wset;
		wset.setDstSet({})
			.setDstBinding(binding)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setPImageInfo(&desc_img);
        cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, m_desc.pipeline.layout, 0, 1, &wset);
		/*
		vk::ImageMemoryBarrier imb_in;
		imb_in.setImage(img.handle)
			.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
			.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
			.setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setSubresourceRange(base_range);
		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eVertexShader,
			vk::PipelineStageFlagBits::eFragmentShader|vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::DependencyFlagBits::eByRegion,
			0, nullptr,
			0, nullptr,
			1, &imb_in);
		*/
	}

	if(m_desc.ubo_buf){
		vk::DescriptorBufferInfo desc_buf {rr.ubo_buf->gpuBuf(), m_desc.ubo_buf.offset, m_desc.ubo_buf.size};
		vk::WriteDescriptorSet wset;
		wset.setDstSet({})
			.setDstBinding(0)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setPBufferInfo(&desc_buf);
        cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, m_desc.pipeline.layout, 0, 1, &wset);
	}
	
	/*{
		vk::ImageMemoryBarrier imb;
		imb.setImage(m_desc.vk_output.handle)
			.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
			.setOldLayout(vk::ImageLayout::eUndefined)
			.setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setSubresourceRange(base_range);
		if(m_desc.blending) {
			imb.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			imb.dstAccessMask |= vk::AccessFlagBits::eColorAttachmentRead;
		}
		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::DependencyFlagBits::eByRegion,
			0, nullptr,
			0, nullptr,
			1, &imb);
	}*/

	vk::RenderPassBeginInfo pass_begin_info;
	pass_begin_info.setRenderPass(m_desc.pipeline.pass)
		.setFramebuffer(m_desc.fb)
		.setClearValueCount(1)
		.setPClearValues(&m_desc.clear_value)
		.renderArea
		.setExtent({outext.width, outext.height})
		.setOffset(vk::Offset2D(0.0f, 0.0f));
	cmd.beginRenderPass(pass_begin_info, vk::SubpassContents::eInline);

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_desc.pipeline.handle);
	vk::Viewport viewport;
	viewport.setX(0).setY(0).setMinDepth(0).setMaxDepth(1).setWidth(outext.width).setHeight(outext.height);
	vk::Rect2D scissor({0, 0}, {outext.width, outext.height});
	cmd.setViewport(0, 1, &viewport);
	cmd.setScissor(0, 1, &scissor);

	for(int i=0;i < m_desc.vertex_bufs.size();i++) {
		auto& buf = m_desc.vertex_bufs[i];
		cmd.bindVertexBuffers(i, 1, &(rr.vertex_buf->gpuBuf()), &buf.offset);
	}
	cmd.draw(4, 1, 0, 0);

	cmd.endRenderPass();
}

void CustomShaderPass::destory(const Device& device, RenderingResources& rr) {
	device.DestroyPipeline(m_desc.pipeline);
	device.handle().destroyFramebuffer(m_desc.fb);
}