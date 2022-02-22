#pragma once
#include "Instance.hpp"
#include "Swapchain.hpp"
#include "vk_mem_alloc.h"

namespace wallpaper
{
namespace vulkan
{

struct RenderingResources {
    vk::Framebuffer framebuffer;
    vk::CommandBuffer command;

	vk::Semaphore sem_swap_wait_image;
	vk::Semaphore sem_swap_finish;
	vk::Fence fence_frame;
};

struct QueueParameters {
	vk::Queue handle;
	uint32_t family_index;

	bool ok() const { return handle; }
};

struct ImageParameters {
	vk::Image handle;
	vk::ImageView view;
	vk::Sampler sampler;
	vk::Extent3D extent;
	VmaAllocation allocation;
	VmaAllocationInfo allocationInfo;

	bool ok() const { return handle; }
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

class Device {
public:
	static vk::ResultValue<Device> Create(Instance&, Span<const char*const> exts);

	void Destroy();

	vk::Result CreateRenderingResource(RenderingResources&);
	void DestroyRenderingResource(RenderingResources&);

	const auto& graphics_queue() const { return m_graphics_queue; }
	const auto& device() const { return m_device; }
	const auto& gpu() const { return m_gpu; }
private:
	std::vector<vk::DeviceQueueCreateInfo> ChooseDeviceQueue(bool present);

	vk::Device m_device;
	vk::PhysicalDevice m_gpu;
	Swapchain m_swapchain;
	VmaAllocator m_allocator; 
	vk::CommandPool m_command_pool;

	QueueParameters m_graphics_queue;
	QueueParameters m_present_queue;
};

}
}