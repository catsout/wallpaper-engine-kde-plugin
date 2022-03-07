#pragma once

#include "Instance.hpp"
#include "Swapchain.hpp"
#include "vk_mem_alloc.h"

namespace wallpaper
{
namespace vulkan
{

struct QueueParameters {
	vk::Queue handle;
	uint32_t family_index;

	bool ok() const { return handle; }
};

struct BufferParameters {
	vk::Buffer handle;
	std::size_t req_size;
	VmaAllocation allocation {};
	VmaAllocationInfo allocationInfo {};
};

struct ImageParameters {
	vk::Image handle;
	vk::ImageView view;
	vk::Sampler sampler;
	vk::Extent3D extent;
	VmaAllocation allocation {};
	VmaAllocationInfo allocationInfo {};

	bool ok() const { return handle; }
};

struct ImageSlots {
	std::vector<ImageParameters> slots;
	int active {0};

	auto& getActive() const { return slots[active]; }
};

struct ExImageParameters {
	vk::Image handle;
	vk::ImageView view;
	vk::Sampler sampler;
	vk::Extent3D extent;
	vk::DeviceMemory mem;
	vk::MemoryRequirements mem_reqs;
	int fd;

	bool ok() const { return handle; }
};
}
}