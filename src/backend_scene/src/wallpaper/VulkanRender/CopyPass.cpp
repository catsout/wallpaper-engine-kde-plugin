#include "CopyPass.hpp"
#include "SpecTexs.h"
#include "Utils/Logging.h"
#include "Utils/AutoDeletor.hpp"
#include "Resource.hpp"

using namespace wallpaper::vulkan;


CopyPass::CopyPass(const Desc& desc):m_desc(desc) {}

CopyPass::~CopyPass() {};

static TextureKey ToTexKey(wallpaper::SceneRenderTarget rt) {
	return TextureKey {
		.width = rt.width,
		.height = rt.height,
		.usage = {},
		.format = wallpaper::TextureFormat::RGBA8,
		.sample = rt.sample
	};
}

void CopyPass::prepare(Scene& scene, const Device& device, RenderingResources& rr) {
    std::array<std::string, 2> textures = {m_desc.src, m_desc.dst};
    std::array<ImageSlots*, 2> vk_textures = {&m_desc.vk_src, &m_desc.vk_dst};
    for(int i=0;i < textures.size();i++) {
        auto& tex_name = textures[i];
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
		if(rv.result != vk::Result::eSuccess) {
            return;
        }  
		*vk_textures[i] = rv.value;
    }

    prepared();
};
void CopyPass::execute(const Device& device, RenderingResources& rr) {
    auto& cmd = rr.command;
    auto& src = m_desc.vk_src.getActive();
    auto& dst = m_desc.vk_dst.getActive();

    vk::ImageSubresourceRange srang;
    srang
        .setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    vk::ImageCopy copy;
    copy.setSrcOffset({})
        .setDstOffset({})
        .setExtent({src.extent.width, src.extent.height, 1});
    copy.srcSubresource
        .setAspectMask(srang.aspectMask)
        .setBaseArrayLayer(0)
        .setLayerCount(1)
        .setMipLevel(1);
    copy.dstSubresource
        .setAspectMask(srang.aspectMask)
        .setBaseArrayLayer(0)
        .setLayerCount(1)
        .setMipLevel(1);
    {
        std::array<vk::ImageMemoryBarrier, 2> imbs;
        imbs[0].setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
            .setDstAccessMask(vk::AccessFlagBits::eTransferRead)
            .setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
            .setImage(src.handle)
            .setSubresourceRange(srang);
        imbs[1].setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
            .setOldLayout(vk::ImageLayout::eUndefined)
            .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
            .setImage(dst.handle)
            .setSubresourceRange(srang);
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer,
                            vk::DependencyFlagBits::eByRegion,
                            0, nullptr,
                            0, nullptr,
                            1, imbs.data());
    }
    cmd.copyImage(
        m_desc.vk_src.getActive().handle,
        vk::ImageLayout::eTransferSrcOptimal,
        m_desc.vk_dst.getActive().handle,
        vk::ImageLayout::eTransferDstOptimal,
        1,
        &copy
    );
    {
        std::array<vk::ImageMemoryBarrier, 2> imbs;
        imbs[0].setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
            .setSrcAccessMask(vk::AccessFlagBits::eTransferRead)
            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
            .setImage(src.handle)
            .setSubresourceRange(srang);
     	imbs[1].setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
            .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setImage(dst.handle)
            .setSubresourceRange(srang);
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                            vk::DependencyFlagBits::eByRegion,
                            0, nullptr,
                            0, nullptr,
                            1, imbs.data());
    }
};
void CopyPass::destory(const Device&, RenderingResources&) {

}