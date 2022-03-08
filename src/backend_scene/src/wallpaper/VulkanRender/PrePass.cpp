#include "PrePass.hpp"
#include "Scene/Scene.h"
#include "Resource.hpp"

using namespace wallpaper::vulkan;

PrePass::PrePass(const Desc&) {}
PrePass::~PrePass() {}

static TextureKey ToTexKey(wallpaper::SceneRenderTarget rt) {
	return TextureKey {
		.width = rt.width,
		.height = rt.height,
		.usage = {},
		.format = wallpaper::TextureFormat::RGBA8,
		.sample = rt.sample
	};
}

//void FinPass::setPresentQueueIndex(uint32_t i) {
//	m_desc.present_queue_index = i;
//}

void PrePass::prepare(Scene& scene, const Device& device, RenderingResources& rr) {
	{
		auto tex_name = std::string(m_desc.result);
		if(scene.renderTargets.count(tex_name) == 0) return;
		auto& rt = scene.renderTargets.at(tex_name);
		auto rv_paras = device.tex_cache().Query(tex_name, ToTexKey(rt));
		m_desc.vk_result = rv_paras.value;
	}
	{
		auto& sc = scene.clearColor;
		m_desc.clear_value = vk::ClearValue(std::array {sc[0], sc[1], sc[2], 1.0f});
	}
	setPrepared();
}

void PrePass::execute(const Device& device, RenderingResources& rr) {
	auto& cmd = rr.command;

	vk::ImageSubresourceRange base_range;
	base_range.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseArrayLayer(0)
		.setBaseMipLevel(0)
		.setLayerCount(VK_REMAINING_ARRAY_LAYERS)
		.setLevelCount(VK_REMAINING_MIP_LEVELS);
	{
		vk::ImageMemoryBarrier imb;
		imb.setImage(m_desc.vk_result.handle)
			.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
			.setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
			.setOldLayout(vk::ImageLayout::eUndefined)
			.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
			.setSubresourceRange(base_range);
		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eTransfer,
			vk::DependencyFlagBits::eByRegion,
			0, nullptr,
			0, nullptr,
			1, &imb);
	}
    cmd.clearColorImage(m_desc.vk_result.handle, vk::ImageLayout::eTransferDstOptimal, &m_desc.clear_value.color, 1, &base_range);

	vk::ImageMemoryBarrier imb;
	imb.setImage(m_desc.vk_result.handle)	
		.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
		.setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
		.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
		.setNewLayout(m_desc.layout)
		.setSubresourceRange(base_range);
	cmd.pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eFragmentShader,
		vk::DependencyFlagBits::eByRegion,
		0, nullptr,
		0, nullptr,
		1, &imb);
}
void PrePass::destory(const Device& device, RenderingResources&) {
}