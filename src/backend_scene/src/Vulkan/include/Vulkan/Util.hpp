#pragma once
#include "Instance.hpp"
#include "Parameters.hpp"
#include "vk_mem_alloc.h"

namespace wallpaper
{
namespace vulkan
{

inline bool CreateStagingBuffer(VmaAllocator allocator, std::size_t size, BufferParameters& buffer) {
	vk::BufferCreateInfo buffInfo;
	buffInfo.setSize(size)
		.setUsage(vk::BufferUsageFlagBits::eTransferSrc);
	buffer.req_size = buffInfo.size;
	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
	VK_CHECK_RESULT_BOOL_RE((vk::Result)vmaCreateBuffer(allocator, (VkBufferCreateInfo *)&buffInfo, &vmaallocInfo,
					(VkBuffer *)&buffer.handle,
					&buffer.allocation,
					&buffer.allocationInfo));
	return true;
}
}
}
