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

static void UpdateUniform(StagingBuffer* buf, 
	const StagingBufferRef& bufref, 
	const ShaderReflected::Block& block, 
	std::string_view name,
	const wallpaper::ShaderValue& value) {
	using namespace wallpaper;
	Span<uint8_t> value_u8 {(uint8_t*)value.data(), value.size()*sizeof(ShaderValue::value_type)};
	auto uni = block.member_map.find(name);
	if(uni == block.member_map.end()) {
		// log
		return;
	}

	size_t offset = uni->second.offset;
	size_t type_size = Sizeof(uni->second.type) * uni->second.num;
	if(type_size != value_u8.size()) {
		//assert(type_size == value_u8.size());
		;//to do
	}
	buf->writeToBuf(bufref, value_u8, offset);
}

void CustomShaderPass::prepare(Scene& scene, const Device& device, RenderingResources& rr) {


    m_desc.vk_textures.resize(m_desc.textures.size());
    for(int i=0;i < m_desc.textures.size();i++) {
        auto& tex_name = m_desc.textures[i];
		if(tex_name.empty()) continue;

		vk::ResultValue<ImageSlots> rv {vk::Result::eIncomplete, {}};
        if(IsSpecTex(tex_name)) {
			if(scene.renderTargets.count(tex_name) == 0) continue;
			auto& rt = scene.renderTargets.at(tex_name);
			auto rv_paras = device.tex_cache().Query(tex_name, ToTexKey(rt), !rt.allowReuse);
			rv.result = rv_paras.result;
			rv.value = ImageSlots{{rv_paras.value}, 0};
		} else {
			auto image = scene.imageParser->Parse(tex_name);
			if(image) {
				rv = device.tex_cache().CreateTex(*image);
			} else {
				LOG_ERROR("parse tex \"%s\" failed", tex_name.c_str());
			}
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
		auto rv = device.tex_cache().Query(tex_name, ToTexKey(rt), !rt.allowReuse);
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
        if(!CompileAndLinkShaderUnits(units, opt, spvs, &ref)) {
			return;
		}

		auto& bindings = descriptor_info.bindings;
		bindings.resize(ref.binding_map.size());
		/*
		LOG_INFO("----shader------");
		LOG_INFO("%s", shader.name.c_str());
		LOG_INFO("--inputs:");
		for(auto& i:ref.input_location_map) {
			LOG_INFO("%d %s", i.second, i.first.c_str());
		}
		LOG_INFO("--bindings:");
		*/
		std::transform(ref.binding_map.begin(), ref.binding_map.end(), bindings.begin(), [](auto& item) {
			//LOG_INFO("%d %s", item.second.binding, item.first.c_str());
			return item.second;
		});

		for(int i=0;i<m_desc.vk_textures.size();i++) {
			int binding {-1};
			if(exists(ref.binding_map, WE_GLTEX_NAMES[i]))
				binding = ref.binding_map.at(WE_GLTEX_NAMES[i]).binding;
			m_desc.vk_tex_binding.push_back(binding);
		}
    }

	m_desc.draw_count = 0;
	std::vector<vk::VertexInputBindingDescription> bind_descriptions;
	std::vector<vk::VertexInputAttributeDescription> attr_descriptions;
	{
		m_desc.dyn_vertex = mesh.Dynamic();
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
				if(!m_desc.dyn_vertex) {
					if(!rr.vertex_buf->allocateSubRef(vertex.CapacitySizeOf(), buf)) return;
					if(!rr.vertex_buf->writeToBuf(buf, {(uint8_t*)vertex.Data(), buf.size})) return;
				} else {
					if(!rr.dyn_buf->allocateSubRef(vertex.CapacitySizeOf(), buf)) return;
				}
			}
			m_desc.draw_count += vertex.DataSize() / vertex.OneSize();
		}

		if(mesh.IndexCount() > 0) {
			auto& indice = mesh.GetIndexArray(0);
			size_t count = (indice.DataCount() * 2)/3;
			m_desc.draw_count = count*3;
			auto& buf = m_desc.index_buf;
			if(!m_desc.dyn_vertex) {
				if(!rr.vertex_buf->allocateSubRef(indice.CapacitySizeof(), buf)) return;
				if(!rr.vertex_buf->writeToBuf(buf, {(uint8_t*)indice.Data(), buf.size})) return;
			} else {
				if(!rr.dyn_buf->allocateSubRef(indice.CapacitySizeof(), buf)) return;
			}
		}
	}
	{
		vk::PipelineColorBlendAttachmentState color_blend;
		vk::AttachmentLoadOp loadOp {vk::AttachmentLoadOp::eDontCare};
		{
			vk::ColorComponentFlags colorMask =
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB;
			bool alpha = !(m_desc.node->Camera().empty() || sstart_with(m_desc.node->Camera(), "global"));
			if(alpha) colorMask |= vk::ColorComponentFlagBits::eA;
			color_blend.setColorWriteMask(colorMask);
			auto blendmode = mesh.Material()->blenmode;
			SetBlend(blendmode, color_blend);
			m_desc.blending = color_blend.blendEnable;

			SetAttachmentLoadOp(blendmode, loadOp);
		}
		auto pass = CreateRenderPass(device.handle(), vk::Format::eR8G8B8A8Unorm, loadOp);
		descriptor_info.push_descriptor = true;
		GraphicsPipeline pipeline;
		pipeline.toDefault();
		pipeline.setRenderPass(pass)
			.addDescriptorSetInfo(descriptor_info)
			.setColorBlendStates(color_blend)
			.setTopology(m_desc.index_buf ? vk::PrimitiveTopology::eTriangleList : vk::PrimitiveTopology::eTriangleStrip)
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
		rr.dyn_buf->allocateSubRef(block.size, m_desc.ubo_buf, device.limits().minUniformBufferOffsetAlignment);
	}

	if(!ref.blocks.empty()) {
		std::function<void()> update_dyn_buf_op;
		if(m_desc.dyn_vertex) {
			auto& mesh = *m_desc.node->Mesh();
			auto* dyn_buf = rr.dyn_buf;
			auto& vertex_bufs = m_desc.vertex_bufs;
			auto& draw_count = m_desc.draw_count;
			auto& index_buf = m_desc.index_buf;
			update_dyn_buf_op = [&mesh, &vertex_bufs, &draw_count, &index_buf, dyn_buf]() {
				if(mesh.Dirty().exchange(false)) {
					for(int i=0;i<mesh.VertexCount();i++) {
						const auto& vertex = mesh.GetVertexArray(i);
						auto& buf = vertex_bufs[i];
						if(!dyn_buf->writeToBuf(buf, {(uint8_t*)vertex.Data(), vertex.DataSizeOf()})) return;
					}
					if(mesh.IndexCount() > 0) {
						auto& indice = mesh.GetIndexArray(0);
						size_t count = (indice.DataCount() * 2)/3;
						draw_count = count*3;
						auto& buf = index_buf;
						if(!dyn_buf->writeToBuf(buf, {(uint8_t*)indice.Data(), indice.DataSizeOf()})) return;
					}
				}
			};
		}

		auto block = ref.blocks.front();
		auto* buf = rr.dyn_buf;
		auto* bufref = &m_desc.ubo_buf;

		auto* node = m_desc.node;
		auto* shader_updater = scene.shaderValueUpdater.get();
		auto& sprites = m_desc.sprites_map;
		auto& vk_textures = m_desc.vk_textures;
		m_desc.update_op = [shader_updater, block, buf, bufref, node, &sprites, &vk_textures, update_dyn_buf_op]() {
			auto update_unf_op = [&block, buf, bufref](std::string_view name, wallpaper::ShaderValue value) {
				UpdateUniform(buf, *bufref, block, name, value);
			};
			auto exists_unf_op = [&block](std::string_view name) {
				return exists(block.member_map, name);
			};

			shader_updater->UpdateUniforms(node, sprites, exists_unf_op, update_unf_op);
			// update image slot for sprites
			{
				for(auto& [i, sp]:sprites) {
					if(i >= vk_textures.size()) continue;
					vk_textures.at(i).active = sp.GetCurFrame().imageId;
				}
			}
			if(update_dyn_buf_op) update_dyn_buf_op();
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
		m_desc.update_op();
	}

	{
		auto& sc = scene.clearColor;
		m_desc.clear_value = vk::ClearValue(std::array {sc[0], sc[1], sc[2], 1.0f});
	}
	for(auto& tex:releaseTexs()) {
		device.tex_cache().MarkShareReady(tex);
	}
    setPrepared();
}

void CustomShaderPass::execute(const Device& device, RenderingResources& rr) {
	if(m_desc.update_op) m_desc.update_op();

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
		auto& img = slot.getActive();
		vk::DescriptorImageInfo desc_img {img.sampler, img.view, vk::ImageLayout::eShaderReadOnlyOptimal};
		vk::WriteDescriptorSet wset;
		wset.setDstSet({})
			.setDstBinding(binding)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setPImageInfo(&desc_img);
        cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, m_desc.pipeline.layout, 0, 1, &wset);
		vk::ImageMemoryBarrier imb;
		imb.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
			.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
			.setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImage(img.handle)
			.setSubresourceRange(base_range);
		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::DependencyFlagBits::eByRegion,
			0, nullptr,
			0, nullptr,
			1, &imb);
	}

	if(m_desc.ubo_buf){
		vk::DescriptorBufferInfo desc_buf {rr.dyn_buf->gpuBuf(), m_desc.ubo_buf.offset, m_desc.ubo_buf.size};
		vk::WriteDescriptorSet wset;
		wset.setDstSet({})
			.setDstBinding(0)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setPBufferInfo(&desc_buf);
        cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, m_desc.pipeline.layout, 0, 1, &wset);
	}

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
	viewport.setX(0).setY(outext.height)
		.setMinDepth(0.0f).setMaxDepth(1.0f)
		.setWidth(outext.width).setHeight(-(float)outext.height);
	vk::Rect2D scissor({0, 0}, {outext.width, outext.height});
	cmd.setViewport(0, 1, &viewport);
	cmd.setScissor(0, 1, &scissor);

	auto& gpu_buf = m_desc.dyn_vertex ? rr.dyn_buf->gpuBuf() : rr.vertex_buf->gpuBuf();

	for(int i=0;i < m_desc.vertex_bufs.size();i++) {
		auto& buf = m_desc.vertex_bufs[i];
		cmd.bindVertexBuffers(i, 1, &gpu_buf, &buf.offset);
	}
	if(m_desc.index_buf) {
		cmd.bindIndexBuffer(gpu_buf, m_desc.index_buf.offset, vk::IndexType::eUint16);
		cmd.drawIndexed(m_desc.draw_count, 1, 0, 0, 0);
	} else {
		cmd.draw(m_desc.draw_count, 1, 0, 0);
	}

	cmd.endRenderPass();
}

void CustomShaderPass::destory(const Device& device, RenderingResources& rr) {
	m_desc.update_op = {};
	{
		auto& buf = m_desc.dyn_vertex ? rr.dyn_buf : rr.vertex_buf;
		for(auto& bufref:m_desc.vertex_bufs) {
			buf->unallocateSubRef(bufref);
		}
	}
	rr.dyn_buf->unallocateSubRef(m_desc.ubo_buf);
	device.DestroyPipeline(m_desc.pipeline);
	device.handle().destroyFramebuffer(m_desc.fb);
}

void CustomShaderPass::setDescTex(uint index, std::string_view tex_key) {
	assert(index < m_desc.textures.size());
	if(index >= m_desc.textures.size()) return;
	m_desc.textures[index] = tex_key;
}