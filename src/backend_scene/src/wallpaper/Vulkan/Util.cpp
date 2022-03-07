#include "Util.hpp"

using namespace wallpaper;

vk::Result vulkan::CreateStagingBuffer(VmaAllocator allocator, std::size_t size, BufferParameters& buffer)
{
	VkResult result;
	do {
		vk::BufferCreateInfo buffInfo;
		buffInfo.setSize(size)
			.setUsage(vk::BufferUsageFlagBits::eTransferSrc);
		buffer.req_size = buffInfo.size;
		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		result = vmaCreateBuffer(allocator, (VkBufferCreateInfo *)&buffInfo, &vmaallocInfo,
						(VkBuffer *)&buffer.handle,
						&buffer.allocation,
						&buffer.allocationInfo);
	} while(false);
	return (vk::Result)result; 
}
