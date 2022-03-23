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

class PipelineParameters;

class Device : NoCopy,NoMove {
public:
	Device();
	~Device();

	static bool Create(Instance&, Span<Extension> exts, vk::Extent2D extent, Device&);
	static bool CheckGPU(vk::PhysicalDevice gpu, Span<Extension> exts, vk::SurfaceKHR surface);

	void Destroy();


	const auto& graphics_queue() const { return m_graphics_queue; }
	const auto& present_queue() const { return m_present_queue; }
	const auto& device() const { return m_device; }
	const auto& handle() const { return m_device; }
	const auto& gpu() const { return m_gpu; }
	const auto& limits() const { return m_limits; }
	const auto& vma_allocator() const { return m_allocator; } 
	const auto& cmd_pool() const { return m_command_pool; }
	const auto& swapchain() const { return m_swapchain; }
	const auto& out_extent() const { return m_extent; }
	void set_out_extent(vk::Extent2D v) { m_extent = v; }

	bool supportExt(std::string_view) const;

	TextureCache& tex_cache() const { return *m_tex_cache; }

	void DestroyBuffer(const BufferParameters&) const;
	void DestroyImageParameters(const ImageParameters& image) const;
	void DestroyPipeline(const PipelineParameters&) const;
	void DestroyExImageParameters(const ExImageParameters& image) const;

	vk::DeviceSize GetUsage() const;
private:
	std::vector<vk::DeviceQueueCreateInfo> ChooseDeviceQueue(vk::SurfaceKHR={});

	vk::Device m_device;
	vk::PhysicalDevice m_gpu;
	vk::PhysicalDeviceLimits m_limits;
	Set<std::string> m_extensions;

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