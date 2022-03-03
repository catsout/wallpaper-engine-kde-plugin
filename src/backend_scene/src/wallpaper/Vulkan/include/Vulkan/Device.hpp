#pragma once
#include "Instance.hpp"
#include "Swapchain.hpp"
#include "vk_mem_alloc.h"
#include "Parameters.hpp"
#include "TextureCache.hpp"

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

class Device : NoCopy,NoMove {
public:
	Device();
	~Device();

	static vk::Result Create(Instance&, Span<const char*const> exts, Device&);

	void Destroy();

	vk::Result CreateRenderingResource(RenderingResources&);
	void DestroyRenderingResource(RenderingResources&);

	const auto& graphics_queue() const { return m_graphics_queue; }
	const auto& device() const { return m_device; }
	const auto& handle() const { return m_device; }
	const auto& gpu() const { return m_gpu; }
	const auto& vma_allocator() const { return m_allocator; } 
	const auto& cmd_pool() const { return m_command_pool; }
	const auto& out_extent() const { return m_extent; }
	void set_out_extent(vk::Extent2D v) { m_extent = v; }

	auto& tex_cache() const { return *m_tex_cache; }
private:
	std::vector<vk::DeviceQueueCreateInfo> ChooseDeviceQueue(vk::SurfaceKHR={});

	vk::Device m_device;
	vk::PhysicalDevice m_gpu;
	Swapchain m_swapchain;
	// c struct
	VmaAllocator m_allocator {}; 
	vk::CommandPool m_command_pool;

	QueueParameters m_graphics_queue;
	QueueParameters m_present_queue;

	// output extent
	vk::Extent2D m_extent {1, 1};

	std::unique_ptr<TextureCache> m_tex_cache;
};

}
}