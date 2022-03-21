#include "CopyPass.hpp"
#include "SpecTexs.h"
#include "Utils/Logging.h"
#include "Utils/AutoDeletor.hpp"
#include "Resource.hpp"
#include "PassCommon.hpp"

using namespace wallpaper::vulkan;


CopyPass::CopyPass(const Desc& desc):m_desc(desc) {}

CopyPass::~CopyPass() {};

void CopyPass::prepare(Scene& scene, const Device& device, RenderingResources& rr) {
    if(scene.renderTargets.count(m_desc.src) == 0) { 
        LOG_ERROR("%s not found", m_desc.src.c_str());
        return;
    }
    if(scene.renderTargets.count(m_desc.dst) == 0) {
        auto& rt = scene.renderTargets.at(m_desc.src);
        scene.renderTargets[m_desc.dst] = rt;
        scene.renderTargets[m_desc.dst].allowReuse = true;
    }
	
    std::array<std::string, 2> textures = {m_desc.src, m_desc.dst};
    std::array<ImageParameters*, 2> vk_textures = {&m_desc.vk_src, &m_desc.vk_dst};
    for(int i=0;i < textures.size();i++) {
        auto& tex_name = textures[i];
		if(tex_name.empty()) continue;

		vk::ResultValue<ImageParameters> rv {vk::Result::eSuccess, {}};
        if(IsSpecTex(tex_name)) {
			auto& rt = scene.renderTargets.at(tex_name);
			rv = device.tex_cache().Query(tex_name, ToTexKey(rt), !rt.allowReuse);
		} else {
            LOG_ERROR("can't copy image source");
            return;
		}
		if(rv.result != vk::Result::eSuccess) {
            return;
        }
		*vk_textures[i] = rv.value;
    }

   	for(auto& tex:releaseTexs()) {
		device.tex_cache().MarkShareReady(tex);
	}

    setPrepared();
};
void CopyPass::execute(const Device& device, RenderingResources& rr) {
    auto& cmd = rr.command;
    auto& src = m_desc.vk_src;
    auto& dst = m_desc.vk_dst;

    if(!(src.handle && dst.handle)) {
        assert(src.handle && dst.handle);
        return;
    }

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
        .setMipLevel(0);
    copy.dstSubresource
        .setAspectMask(srang.aspectMask)
        .setBaseArrayLayer(0)
        .setLayerCount(1)
        .setMipLevel(0);
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
                            2, imbs.data());
    }
    cmd.copyImage(
        src.handle,
        vk::ImageLayout::eTransferSrcOptimal,
        dst.handle,
        vk::ImageLayout::eTransferDstOptimal,
        1,
        &copy
    );
    {
        std::array<vk::ImageMemoryBarrier, 2> imbs;
        imbs[0]
            .setSrcAccessMask(vk::AccessFlagBits::eTransferRead)
            .setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
            .setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setImage(src.handle)
            .setSubresourceRange(srang);
     	imbs[1]
            .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
            .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setImage(dst.handle)
            .setSubresourceRange(srang);
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                            vk::DependencyFlagBits::eByRegion,
                            0, nullptr,
                            0, nullptr,
                            2, imbs.data());
    }

    if(dst.mipmap_level > 1) {
        device.tex_cache().RecGenerateMipmaps(cmd, dst);
    }
};
void CopyPass::destory(const Device&, RenderingResources&) {

}