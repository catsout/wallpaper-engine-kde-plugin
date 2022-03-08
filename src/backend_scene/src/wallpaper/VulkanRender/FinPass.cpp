#include "FinPass.hpp"
#include "Vulkan/Shader.hpp"
#include "Resource.hpp"

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

constexpr std::array<VertexInput, 4> vertex_input = {
	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 0.0f, 1.0f, 1.0f
};


FinPass::FinPass(const Desc&) {}
FinPass::~FinPass() {}

static vk::RenderPass CreateRenderPass(const vk::Device &device, vk::Format format)
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


static TextureKey ToTexKey(wallpaper::SceneRenderTarget rt) {
	return TextureKey {
		.width = rt.width,
		.height = rt.height,
		.usage = {},
		.format = wallpaper::TextureFormat::RGBA8,
		.sample = rt.sample
	};
}

void FinPass::setPresent(ImageParameters img) {
	m_desc.vk_present = img;
}
void FinPass::setPresentLayout(vk::ImageLayout layout) {
	m_desc.present_layout = layout;
}
void FinPass::setPresentFormat(vk::Format format) {
	m_desc.present_format = format;
}
void FinPass::setPresentQueueIndex(uint32_t i) {
	m_desc.present_queue_index = i;
}

void FinPass::prepare(Scene& scene, const Device& device, RenderingResources& rr) {
	{
		auto tex_name = std::string(m_desc.result);
		if(scene.renderTargets.count(tex_name) == 0) return;
		auto& rt = scene.renderTargets.at(tex_name);
		auto rv_paras = device.tex_cache().Query(tex_name, ToTexKey(rt));
		m_desc.vk_result = rv_paras.value;
	}
	std::vector<Uni_ShaderSpv> spvs;
    {
        ShaderCompOpt opt;
        opt.client_ver = glslang::EShTargetVulkan_1_1;
        opt.relaxed_errors_glsl = true;
        opt.relaxed_rules_vulkan = true;
        opt.suppress_warnings_glsl = true;

        std::array<ShaderCompUnit, 2> units;
        units[0] = ShaderCompUnit{.stage=EShLangVertex, .src=std::string(vert_code)};
        units[1] = ShaderCompUnit{.stage=EShLangFragment, .src=std::string(frag_code)};
        CompileAndLinkShaderUnits(units, opt, spvs, nullptr);
    }

	vk::VertexInputBindingDescription bind_description;
	std::vector<vk::VertexInputAttributeDescription> attr_descriptions;
	{
		bind_description.setStride(sizeof(VertexInput)).setInputRate(vk::VertexInputRate::eVertex).setBinding(0);
		vk::VertexInputAttributeDescription attr_pos, attr_color;
		attr_pos.setBinding(0)
			.setLocation(0)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setOffset(offsetof(VertexInput, pos));
		attr_color.setBinding(0)
			.setLocation(1)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setOffset(offsetof(VertexInput, color));
		
		attr_descriptions.push_back(attr_pos);
		attr_descriptions.push_back(attr_color);

		{
			auto& buf = m_desc.vertex_buf;
			rr.vertex_buf->allocateSubRef(sizeof(decltype(vertex_input)), buf);
			rr.vertex_buf->writeToBuf(buf, {(uint8_t*)vertex_input.data(), buf.size});
		}
	}
	DescriptorSetInfo descriptor_info;
	{
		descriptor_info.push_descriptor = true;
		descriptor_info.bindings.resize(1);
		auto& binding = descriptor_info.bindings.back();
		binding.setBinding(1)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment);
	}
	{
		auto pass = CreateRenderPass(device.handle(), m_desc.present_format);
		descriptor_info.push_descriptor = true;
		GraphicsPipeline pipeline;
		pipeline.toDefault();
		pipeline.setRenderPass(pass)
			.addDescriptorSetInfo(descriptor_info)
			.setTopology(vk::PrimitiveTopology::eTriangleStrip)
			.addInputBindingDescription(bind_description)
			.addInputAttributeDescription(attr_descriptions);
		for(auto& spv:spvs) pipeline.addStage(std::move(spv));

		pipeline.create(device, m_desc.pipeline);
	}
	if(m_desc.present_layout == vk::ImageLayout::ePresentSrcKHR || m_desc.present_layout == vk::ImageLayout::eSharedPresentKHR)
		m_desc.render_layout = m_desc.present_layout;
	else m_desc.render_layout = vk::ImageLayout::eColorAttachmentOptimal;

	{
		auto& sc = scene.clearColor;
		m_desc.clear_value = vk::ClearValue(std::array {sc[0], sc[1], sc[2], 1.0f});
	}
	setPrepared();
}

void FinPass::execute(const Device& device, RenderingResources& rr) {
	auto& cmd = rr.command;
	auto& outext = m_desc.vk_present.extent;

	vk::ImageSubresourceRange base_range;
	base_range.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseArrayLayer(0)
		.setBaseMipLevel(0)
		.setLayerCount(VK_REMAINING_ARRAY_LAYERS)
		.setLevelCount(VK_REMAINING_MIP_LEVELS);

	{
		if(m_desc.fb) {
			device.handle().destroyFramebuffer(m_desc.fb);
		}
		vk::FramebufferCreateInfo info;
		info.setRenderPass(m_desc.pipeline.pass)
			.setAttachmentCount(1)
			.setPAttachments(&m_desc.vk_present.view)
			.setWidth(m_desc.vk_present.extent.width)
			.setHeight(m_desc.vk_present.extent.height)
			.setLayers(1);
	 	(void)device.handle().createFramebuffer(&info, nullptr, &m_desc.fb);
	}
	{
		vk::DescriptorImageInfo desc_img {m_desc.vk_result.sampler, m_desc.vk_result.view, vk::ImageLayout::eShaderReadOnlyOptimal};
		vk::WriteDescriptorSet wset;
		wset.setDstSet({})
			.setDstBinding(1)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setPImageInfo(&desc_img);
        cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, m_desc.pipeline.layout, 0, 1, &wset);
	}

	{
		vk::ImageMemoryBarrier imb;
		imb.setImage(m_desc.vk_present.handle)
			.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
			.setOldLayout(vk::ImageLayout::eUndefined)
			.setNewLayout(m_desc.render_layout)
			.setSrcQueueFamilyIndex(m_desc.present_queue_index)
			.setDstQueueFamilyIndex(device.graphics_queue().family_index)
			.setSubresourceRange(base_range);
		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::DependencyFlagBits::eByRegion,
			0, nullptr,
			0, nullptr,
			1, &imb);
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
	viewport.setX(0).setY(0).setMinDepth(0).setMaxDepth(1).setWidth(outext.width).setHeight(outext.height);
	vk::Rect2D scissor({0, 0}, {outext.width, outext.height});
	cmd.setViewport(0, 1, &viewport);
	cmd.setScissor(0, 1, &scissor);

	cmd.bindVertexBuffers(0, 1, &(rr.vertex_buf->gpuBuf()), &m_desc.vertex_buf.offset);
	cmd.draw(4, 1, 0, 0);
	cmd.endRenderPass();	

	vk::ImageMemoryBarrier imb;
	imb.setImage(m_desc.vk_present.handle)	
		.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
		.setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
		.setOldLayout(m_desc.render_layout)
		.setNewLayout(m_desc.present_layout)
		.setSrcQueueFamilyIndex(device.graphics_queue().family_index)
		.setDstQueueFamilyIndex(m_desc.present_queue_index)
		.setSubresourceRange(base_range);
	cmd.pipelineBarrier(
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::DependencyFlagBits::eByRegion,
		0, nullptr,
		0, nullptr,
		1, &imb);
}
void FinPass::destory(const Device& device, RenderingResources&) {
	device.DestroyPipeline(m_desc.pipeline);
	device.handle().destroyFramebuffer(m_desc.fb);
}