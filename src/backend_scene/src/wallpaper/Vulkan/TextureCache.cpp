
#include "TextureCache.hpp"
#include <cstdio>

using namespace wallpaper::vulkan;

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
			.setPNext(&ex_info);
		info.extent.setWidth(width)
			.setHeight(height)
			.setDepth(1);
	
		image.extent = info.extent;
		rv.result = device.createImage(&info, nullptr, &image.handle);
		if(rv.result != vk::Result::eSuccess) break;

		image.mem_reqs = device.getImageMemoryRequirements(image.handle);
		auto rv_memory = AllocateMemory(device, gpu, image.mem_reqs, 
			vk::MemoryPropertyFlagBits::eDeviceLocal, &ex_mem_info);
		rv.result = rv_memory.result;
		if(rv.result != vk::Result::eSuccess) break;
		image.mem = rv_memory.value;

		rv.result = device.bindImageMemory(image.handle, rv_memory.value, 0);
		if(rv.result != vk::Result::eSuccess) break;
		else rv.result = vk::Result::eIncomplete;

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
			if(rv.result != vk::Result::eSuccess) break;
		}
		rv.result = device.createSampler(&sampler_info, nullptr, &image.sampler);
		if(rv.result != vk::Result::eSuccess) break;
		{
			vk::MemoryGetFdInfoKHR info;
			info.setMemory(image.mem)
				.setHandleType(vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd);
			rv.result = device.getMemoryFdKHR(&info, &image.fd);
			if(rv.result != vk::Result::eSuccess) break;
		}
		rv.result = vk::Result::eSuccess;
	} while(false);
	return rv;
}


vk::ResultValue<ExImageParameters> TextureCache::CreateExTex(uint32_t width, uint32_t height, vk::Format format) {
	vk::SamplerCreateInfo sampler_info;
	sampler_info.setMagFilter(vk::Filter::eLinear)
		.setMinFilter(vk::Filter::eLinear)
		.setMipmapMode(vk::SamplerMipmapMode::eNearest)
		.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
		.setAnisotropyEnable(false)
		.setMaxAnisotropy(1.0f)
		.setCompareEnable(false)
		.setCompareOp(vk::CompareOp::eAlways)
		.setMinLod(0.0f).setMaxLod(0.0f)
		.setBorderColor(vk::BorderColor::eFloatTransparentBlack)
		.setUnnormalizedCoordinates(false);
    return CreateExImage(width, height, format, sampler_info,
        vk::ImageUsageFlagBits::eSampled | 
		vk::ImageUsageFlagBits::eColorAttachment |
		vk::ImageUsageFlagBits::eTransferDst, 
		m_device.device(), m_device.gpu());
}