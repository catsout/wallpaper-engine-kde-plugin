
#include "TextureCache.hpp"
#include "Device.hpp"
#include "Util.hpp"

#include "Image.h"
#include "Utils/MapSet.hpp"
#include "Utils/AutoDeletor.hpp"
#include "Utils/Hash.h"
#include <cstdio>

using namespace wallpaper::vulkan;

vk::Format wallpaper::vulkan::ToVkType(TextureFormat tf) {
	switch (tf)
	{
	case TextureFormat::BC1:
		return vk::Format::eBc1RgbaUnormBlock;
	case TextureFormat::BC2:
		return vk::Format::eBc2UnormBlock;
	case TextureFormat::BC3:
		return vk::Format::eBc3UnormBlock;
	case TextureFormat::R8:
		return vk::Format::eR8Unorm;
	case TextureFormat::RG8:
		return vk::Format::eR8G8Unorm;
	case TextureFormat::RGB8:
		return vk::Format::eR8G8B8Unorm;
	case TextureFormat::RGBA8:
		return vk::Format::eR8G8B8A8Unorm;
	default:
		assert(false);
		return vk::Format::eR8G8B8A8Unorm;
	}
}

std::size_t TextureKey::HashValue(const TextureKey& k) {
	std::size_t seed {0};
	utils::hash_combine(seed, k.width);
	utils::hash_combine(seed, k.height);
	utils::hash_combine(seed, (int)k.usage);
	utils::hash_combine(seed, (int)k.format);
	utils::hash_combine(seed, (int)k.mipmap_level);

	utils::hash_combine(seed, (int)k.sample.wrapS);
	utils::hash_combine(seed, (int)k.sample.wrapT);
	utils::hash_combine(seed, (int)k.sample.magFilter);
	return seed;
}


vk::SamplerAddressMode wallpaper::vulkan::ToVkType(wallpaper::TextureWrap sam) {
	using namespace wallpaper;
	switch (sam)
	{
	case TextureWrap::CLAMP_TO_EDGE:
		return vk::SamplerAddressMode::eClampToEdge;
	case TextureWrap::REPEAT:
	default:
		return vk::SamplerAddressMode::eRepeat;
	}	
}
vk::Filter wallpaper::vulkan::ToVkType(wallpaper::TextureFilter sam) {
	using namespace wallpaper;
	switch (sam)
	{
	case TextureFilter::LINEAR:
		return vk::Filter::eLinear;
	case TextureFilter::NEAREST:
	default:
		return vk::Filter::eNearest;
	}	
}

static vk::SamplerCreateInfo GenSamplerInfo(TextureKey key) {
	vk::SamplerCreateInfo sampler_info;
	auto& sam = key.sample;
	sampler_info
		.setMagFilter(ToVkType(sam.magFilter))
		.setMinFilter(ToVkType(sam.minFilter))
		.setAddressModeU(ToVkType(sam.wrapS))
		.setAddressModeV(ToVkType(sam.wrapS))
		.setAddressModeW(ToVkType(sam.wrapT))
		.setMipmapMode(vk::SamplerMipmapMode::eLinear)
		.setAnisotropyEnable(false)
		.setMaxAnisotropy(1.0f)
		.setCompareEnable(false)
		.setCompareOp(vk::CompareOp::eNever)
		.setMinLod(0.0f)
		.setMaxLod(1.0f)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(false);
	return sampler_info;
}

static vk::Result TransImgLayout(const vk::Queue &queue, vk::CommandBuffer &cmd, const ImageParameters &image, vk::ImageLayout layout)
{
	vk::Result result;
	do {
		result = cmd.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
		if(result != vk::Result::eSuccess) break;

		vk::ImageSubresourceRange subresourceRange;
		subresourceRange
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseMipLevel(0)
			.setBaseArrayLayer(0)
			.setLevelCount(VK_REMAINING_MIP_LEVELS)
			.setLayerCount(VK_REMAINING_ARRAY_LAYERS);

		{
			vk::ImageMemoryBarrier out_bar;
			out_bar.setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite)
				.setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
				.setOldLayout(vk::ImageLayout::eUndefined)
				.setNewLayout(layout)
				.setImage(image.handle)
				.setSubresourceRange(subresourceRange);
			cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
								vk::DependencyFlagBits::eByRegion,
								0, nullptr,
								0, nullptr,
								1, &out_bar);
		}
		result = cmd.end();
		if(result != vk::Result::eSuccess) break;

		vk::SubmitInfo sub_info;
		sub_info.setCommandBufferCount(1)
			.setPCommandBuffers(&cmd);
		result = queue.submit(1, &sub_info, {});
	} while(false);
	return result;
}


static vk::ResultValue<vk::DeviceMemory> AllocateMemory(const vk::Device& device, const vk::PhysicalDevice& gpu, vk::MemoryRequirements reqs,
	vk::MemoryPropertyFlags property, void* pNext=NULL) {
	vk::PhysicalDeviceMemoryProperties pros = gpu.getMemoryProperties();
	for( uint32_t i = 0; i < pros.memoryTypeCount; ++i ) {
  		if((reqs.memoryTypeBits & (1 << i)) &&
    	(pros.memoryTypes[i].propertyFlags & property)) {
			vk::MemoryAllocateInfo memory_allocate_info; 
			memory_allocate_info.setAllocationSize(reqs.size)
				.setMemoryTypeIndex(i)
				.setPNext(pNext);
			return device.allocateMemory(memory_allocate_info);
		}
	}
	// to do
	return {vk::Result::eIncomplete,{}};
}

static vk::ResultValue<ExImageParameters> CreateExImage(uint32_t width, uint32_t height, vk::Format format,
	vk::SamplerCreateInfo sampler_info,
	vk::ImageUsageFlags usage,
	const vk::Device& device,
	const vk::PhysicalDevice& gpu) {

	vk::ResultValue<ExImageParameters> rv {vk::Result::eIncomplete, {}};
	auto& image = rv.value;

	do {
		vk::ImageCreateInfo info;
		VkExternalMemoryImageCreateInfo ex_info {
			.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
			.pNext = NULL,
			.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT
		};
		VkExportMemoryAllocateInfo ex_mem_info {
			.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO,
			.pNext = NULL,
			.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT
		};
		info.setImageType(vk::ImageType::e2D)
			.setFormat(format)
			.setArrayLayers(1)
			.setMipLevels(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(usage)
			.setQueueFamilyIndexCount(0)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setPNext(&ex_info);
		info.extent.setWidth(width)
			.setHeight(height)
			.setDepth(1);
	
		image.extent = info.extent;
		rv.result = device.createImage(&info, nullptr, &image.handle);
		VK_CHECK_RESULT_ACT(break, rv.result);

		image.mem_reqs = device.getImageMemoryRequirements(image.handle);
		auto rv_memory = AllocateMemory(device, gpu, image.mem_reqs, 
			vk::MemoryPropertyFlagBits::eDeviceLocal, &ex_mem_info);
		rv.result = rv_memory.result;
		VK_CHECK_RESULT_ACT(break, rv.result);
		image.mem = rv_memory.value;

		rv.result = device.bindImageMemory(image.handle, rv_memory.value, 0);
		VK_CHECK_RESULT_ACT(break, rv.result);

		{
			vk::ImageViewCreateInfo createinfo;
			createinfo
				.setFormat(format)
				.setImage(image.handle)
				.setViewType(vk::ImageViewType::e2D);
			createinfo.subresourceRange
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setBaseArrayLayer(0)
				.setLayerCount(1);
			rv.result = device.createImageView(&createinfo, nullptr, &image.view);
			VK_CHECK_RESULT_ACT(break, rv.result);
		}
		rv.result = device.createSampler(&sampler_info, nullptr, &image.sampler);
		VK_CHECK_RESULT_ACT(break, rv.result);
		{
			vk::MemoryGetFdInfoKHR info;
			info.setMemory(image.mem)
				.setHandleType(vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd);
			rv.result = device.getMemoryFdKHR(&info, &image.fd);
			VK_CHECK_RESULT_ACT(break, rv.result);
		}
		rv.result = vk::Result::eSuccess;
	} while(false);
	return rv;
}


vk::ResultValue<ExImageParameters> TextureCache::CreateExTex(uint32_t width, uint32_t height, vk::Format format) {
	vk::SamplerCreateInfo sampler_info;
	sampler_info
		.setMagFilter(vk::Filter::eNearest)
		.setMinFilter(vk::Filter::eNearest)
		.setMipmapMode(vk::SamplerMipmapMode::eLinear)
		.setAddressModeU(vk::SamplerAddressMode::eRepeat)
		.setAddressModeV(vk::SamplerAddressMode::eRepeat)
		.setAddressModeW(vk::SamplerAddressMode::eRepeat)
		.setAnisotropyEnable(false)
		.setMaxAnisotropy(1.0f)
		.setCompareEnable(false)
		.setCompareOp(vk::CompareOp::eNever)
		.setMinLod(0.0f).setMaxLod(1.0f)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(false);
    auto rv = CreateExImage(width, height, format, sampler_info,
        vk::ImageUsageFlagBits::eSampled |
		vk::ImageUsageFlagBits::eColorAttachment |
		vk::ImageUsageFlagBits::eTransferDst, 
		m_device.device(), m_device.gpu());
	VK_CHECK_RESULT(rv.result);
	auto& eximg = rv.value;

	if(!m_tex_cmd) allocateCmd();
	TransImgLayout(m_device.graphics_queue().handle, m_tex_cmd, eximg.toImageParameters(), vk::ImageLayout::eGeneral);
	rv.result = m_device.handle().waitIdle();
	VK_CHECK_RESULT(rv.result);
	return rv;
}

static vk::Result CreateImage(const Device &device, ImageParameters& image, 
							vk::Extent3D extent, uint miplevel, vk::Format format,
							vk::SamplerCreateInfo sampler_info,
							vk::ImageUsageFlags usage,
							VmaMemoryUsage mem_usage = VMA_MEMORY_USAGE_GPU_ONLY)
{
	vk::Result result;
	do {
		vk::ImageCreateInfo info;
		info.setImageType(vk::ImageType::e2D)
			.setFormat(format)
			.setArrayLayers(1)
			.setMipLevels(miplevel)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(usage)
			.setQueueFamilyIndexCount(0)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setSharingMode(vk::SharingMode::eExclusive);
		info.extent = extent;

		image.extent = info.extent;
		VmaAllocationCreateInfo vma_info{};
		vma_info.usage = mem_usage;
		result = (vk::Result)vmaCreateImage(device.vma_allocator(),
					(VkImageCreateInfo *)&info,
					&vma_info,
					(VkImage *)&image.handle,
					&image.allocation, &image.allocationInfo);
		if(result != vk::Result::eSuccess) break;
		image.mipmap_level = miplevel;
		{
			vk::ImageViewCreateInfo createinfo;
			createinfo
				.setFormat(format)
				.setImage(image.handle)
				.setViewType(vk::ImageViewType::e2D);
			createinfo.subresourceRange
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseMipLevel(0)
				.setLevelCount(miplevel)
				.setBaseArrayLayer(0)
				.setLayerCount(1);
			result = device.handle().createImageView(&createinfo, nullptr, &image.view);
			if(result != vk::Result::eSuccess) break;
		}
		result = device.handle().createSampler(&sampler_info, nullptr, &image.sampler);
	} while(false);
	if(result != vk::Result::eSuccess) {
		device.DestroyImageParameters(image);
	}
	return result;
}


static vk::Result CopyImageData(Span<BufferParameters> in_bufs, Span<vk::Extent3D> in_exts, const vk::Queue &queue, vk::CommandBuffer &cmd,
				ImageParameters &image)
{
	vk::Result result;
	do {
		result = cmd.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
		if(result != vk::Result::eSuccess) break;

		vk::ImageSubresourceRange subresourceRange;
		subresourceRange
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseMipLevel(0)
			.setLevelCount(in_bufs.size())
			.setBaseArrayLayer(0)
			.setLayerCount(1);

		{
			vk::ImageMemoryBarrier in_bar;
			in_bar.setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
				.setOldLayout(vk::ImageLayout::eUndefined)
				.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
				.setImage(image.handle)
				.setSubresourceRange(subresourceRange);
			cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer,
								vk::DependencyFlagBits::eByRegion,
								0, nullptr,
								0, nullptr,
								1, &in_bar);
		}
		vk::BufferImageCopy copy;
		copy.imageSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setLayerCount(1);
		for(int i=0; i<in_bufs.size() ;i++) { 
			copy.imageSubresource.setMipLevel(i);
			copy.setImageExtent(in_exts[i]);
			cmd.copyBufferToImage(in_bufs[i].handle, image.handle, vk::ImageLayout::eTransferDstOptimal, 1, &copy);
		}
		{
			vk::ImageMemoryBarrier out_bar;
			out_bar.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
				.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
				.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
				.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setImage(image.handle)
				.setSubresourceRange(subresourceRange);
			cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
								vk::DependencyFlagBits::eByRegion,
								0, nullptr,
								0, nullptr,
								1, &out_bar);
		}
		result = cmd.end();
		if(result != vk::Result::eSuccess) break;

		vk::SubmitInfo sub_info;
		sub_info.setCommandBufferCount(1)
			.setPCommandBuffers(&cmd);
		result = queue.submit(1, &sub_info, {});
	} while(false);
	return result;
}



vk::ResultValue<ImageSlots> TextureCache::CreateTex(Image& image) {
	vk::ResultValue<ImageSlots> rv {vk::Result::eIncomplete, {}};

	if(exists(m_tex_map, image.key)) {
		rv.result = vk::Result::eSuccess;
		rv.value = m_tex_map.at(image.key);
		return rv;
	}

	if(!m_tex_cmd) allocateCmd();

	rv.value.slots.resize(image.slots.size());

	auto& sam = image.header.sample;

	for(int i=0; i<image.slots.size();i++) {
		auto& image_paras = rv.value.slots[i];
		auto& image_slot = image.slots[i];
		auto mipmap_levels = image_slot.mipmaps.size();

		// check data
		if(!image_slot) return rv;

		vk::SamplerCreateInfo sampler_info;
		sampler_info
			.setMagFilter(ToVkType(sam.magFilter))
			.setMinFilter(ToVkType(sam.minFilter))
			.setAddressModeU(ToVkType(sam.wrapS))
			.setAddressModeV(ToVkType(sam.wrapS))
			.setAddressModeW(ToVkType(sam.wrapT))
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setAnisotropyEnable(false)
			.setMaxAnisotropy(1.0f)
			.setCompareEnable(false)
			.setCompareOp(vk::CompareOp::eNever)
			.setMinLod(0.0f)
			.setMaxLod(mipmap_levels)
			.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
			.setUnnormalizedCoordinates(false);
		
		vk::Format format = ToVkType(image.header.format);
		vk::Extent3D ext {image_slot.width, image_slot.height, 1};
		
		rv.result = CreateImage(m_device, image_paras, ext, mipmap_levels, format, sampler_info, 	
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);	
		VK_CHECK_RESULT_ACT(break, rv.result);

		std::vector<BufferParameters> stage_bufs;
		std::vector<vk::Extent3D> extents;
		for(int j=0;j<image_slot.mipmaps.size();j++) {
			auto& image_data = image_slot.mipmaps[j];
			BufferParameters buf;
			(void)CreateStagingBuffer(m_device.vma_allocator(), image_data.size, buf);
			{
				void *v_data;
				(void)vmaMapMemory(m_device.vma_allocator(), buf.allocation, &v_data);
				memcpy(v_data, image_data.data.get(), image_data.size);
				vmaUnmapMemory(m_device.vma_allocator(), buf.allocation);
			}
			stage_bufs.push_back(buf);
			extents.emplace_back(image_data.width, image_data.height, 1);
		}
		rv.result = CopyImageData(stage_bufs, extents, m_device.graphics_queue().handle, m_tex_cmd, image_paras);
		AUTO_DELETER(stage, ([&stage_bufs, this]() {
			std::for_each(stage_bufs.begin(), stage_bufs.end(), [this](auto& buf) {
				vmaDestroyBuffer(m_device.vma_allocator(), buf.handle, buf.allocation);
			});
		}));
		VK_CHECK_RESULT_ACT(break, rv.result);
		rv.result = m_device.handle().waitIdle();
	}
	m_tex_map[image.key] = rv.value;
	return rv;
}

void TextureCache::allocateCmd() {
	vk::CommandPool pool = m_device.cmd_pool();
	m_tex_cmd = m_device.handle().allocateCommandBuffers({pool, vk::CommandBufferLevel::ePrimary, 1}).value.front();
}

vk::ResultValue<ImageParameters> TextureCache::CreateTex(TextureKey tex_key) {
	vk::ResultValue<ImageParameters> rv {vk::Result::eIncomplete, {}}; 
	auto& image_paras = rv.value;
	do {
		vk::SamplerCreateInfo sam_info = GenSamplerInfo(tex_key);
		vk::Format format = ToVkType(tex_key.format);
		vk::Extent3D ext {tex_key.width, tex_key.height, 1};

		rv.result = CreateImage(m_device, image_paras, ext, tex_key.mipmap_level, format, sam_info, 	
			vk::ImageUsageFlagBits::eTransferSrc | 
			vk::ImageUsageFlagBits::eTransferDst | 
			vk::ImageUsageFlagBits::eSampled |
			vk::ImageUsageFlagBits::eColorAttachment);	
		VK_CHECK_RESULT_ACT(break, rv.result);

		if(!m_tex_cmd) allocateCmd();
		TransImgLayout(m_device.graphics_queue().handle, m_tex_cmd, image_paras, vk::ImageLayout::eShaderReadOnlyOptimal);
		rv.result = m_device.handle().waitIdle();
		VK_CHECK_RESULT_ACT(break, rv.result);
	} while(false);
	return rv;
}

TextureCache::TextureCache(const Device& device):m_device(device) {}

TextureCache::~TextureCache() {};


void TextureCache::Clear() {
	for(auto& texs:m_tex_map) {
		auto& image_slot = texs.second;
		for(auto& image:image_slot.slots) {
			m_device.DestroyImageParameters(image);
		}
	}
	m_tex_map.clear();
	for(auto& query:m_query_texs) {
		m_device.DestroyImageParameters(query->image);
	}
	m_query_texs.clear();
	m_query_map.clear();
}

void TextureCache::Destroy() {
	Clear();
	m_device.handle().freeCommandBuffers(m_device.cmd_pool(), 1, &m_tex_cmd);
}

vk::ResultValue<ImageParameters> TextureCache::Query(std::string_view key, TextureKey content_hash, bool persist) {
	vk::ResultValue<ImageParameters> rv {vk::Result::eSuccess, {}};
	if(exists(m_query_map, key)) {
		auto& query = *(m_query_map.find(key)->second);

		query.share_ready = false;
		query.persist = persist;
		rv.value = query.image;
		return rv;
	};

	TexHash tex_hash = TextureKey::HashValue(content_hash);
	for(auto& query:m_query_texs) {
		if(!(query->share_ready)) continue;
		if(query->content_hash != tex_hash) continue;

		query->share_ready = false;
		query->persist = persist;
		query->query_keys.insert(std::string(key));
		rv.value = query->image;

		m_query_map[std::string(key)] = &(*query);
		return rv;
	}

	m_query_texs.emplace_back(std::make_unique<QueryTex>());
	auto& query = *m_query_texs.back();
	m_query_map[std::string(key)] = &query;

	query.index = m_query_texs.size() - 1;
	query.content_hash = tex_hash;
	query.query_keys.insert(std::string(key));
	auto rv_image = CreateTex(content_hash);
	rv.result = rv_image.result;
	query.image = rv_image.value;
	query.persist = persist;
	rv.value = query.image;
	return rv;
}

void TextureCache::MarkShareReady(std::string_view key) {
	if(exists(m_query_map, key)) {
		auto& query = m_query_map.find(key)->second;
		if(query->persist) return;
		query->share_ready = true;
		m_query_map.erase(key.data());
	}
}


void TextureCache::RecGenerateMipmaps(vk::CommandBuffer& cmd, const ImageParameters& image) const {
	vk::ImageMemoryBarrier barrier;
	barrier.image = image.handle;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	uint16_t mipWidth = image.extent.width;
	uint16_t mipHeight = image.extent.height;

	for (uint i = 1; i < image.mipmap_level; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = i==1 ? vk::ImageLayout::eShaderReadOnlyOptimal : vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eMemoryRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
			vk::DependencyFlagBits::eByRegion,
			0, nullptr,
			0, nullptr,
			1, &barrier);
		barrier.subresourceRange.baseMipLevel = i;
		barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
			vk::DependencyFlagBits::eByRegion,
			0, nullptr,
			0, nullptr,
			1, &barrier);


		vk::ImageBlit blit;
		blit.srcOffsets[0] = vk::Offset3D{0, 0, 0};
		blit.srcOffsets[1] = vk::Offset3D{mipWidth, mipHeight, 1};
		blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = vk::Offset3D{0, 0, 0};
		blit.dstOffsets[1] = vk::Offset3D{ mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = blit.srcSubresource.aspectMask;
		blit.dstSubresource.mipLevel = blit.srcSubresource.mipLevel + 1;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

	 	cmd.blitImage(
			image.handle, vk::ImageLayout::eTransferSrcOptimal,
			image.handle, vk::ImageLayout::eTransferDstOptimal,
			1, &blit,
			vk::Filter::eLinear);

		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
			vk::DependencyFlagBits::eByRegion,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = image.mipmap_level - 1;
	barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
	barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
	cmd.pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
		vk::DependencyFlagBits::eByRegion,
		0, nullptr,
		0, nullptr,
		1, &barrier);
}
