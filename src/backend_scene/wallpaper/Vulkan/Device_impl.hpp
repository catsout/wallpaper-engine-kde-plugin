#include "Device.hpp"

#include "Utils/Logging.h"
#include "GraphicsPipeline.hpp"

using namespace wallpaper::vulkan;

namespace {

void enumateDeviceExts(const vk::PhysicalDevice& gpu, wallpaper::Set<std::string>& set) {
	auto rv = gpu.enumerateDeviceExtensionProperties();
	VK_CHECK_RESULT_VOID_RE(rv.result);
	for(auto& ext:rv.value) {
		set.insert(ext.extensionName);
	}
}

vk::Result CreateDevice(Instance& instance,
	Span<vk::DeviceQueueCreateInfo> queueCreateInfos,
	Span<std::string_view> deviceExts,
	vk::Device* device) {
	
	std::vector<const char*> exts_names_c;
	std::transform(deviceExts.begin(), deviceExts.end(), std::back_inserter(exts_names_c), [](auto& s) {
		return s.data();
	});
	
	vk::DeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo
		.setQueueCreateInfoCount(queueCreateInfos.size())
		.setPQueueCreateInfos(queueCreateInfos.data())
		.setEnabledExtensionCount(exts_names_c.size())
		.setPpEnabledExtensionNames(exts_names_c.data());

    return instance.gpu().createDevice(&deviceCreateInfo, nullptr, device); 
}

}


bool Device::CheckGPU(vk::PhysicalDevice gpu, Span<Extension> exts, vk::SurfaceKHR surface) {
	std::vector<vk::DeviceQueueCreateInfo> queues;
	auto props = gpu.getQueueFamilyProperties();

	// check queue
	bool has_graphics_queue {false};
	bool has_present_queue {false};
	uint index {0};
	for(auto& prop:props) {
		if(prop.queueFlags & vk::QueueFlagBits::eGraphics)
			has_graphics_queue = true;
		if(surface) {
			vk::Bool32 ok {false};
			VK_CHECK_RESULT(gpu.getSurfaceSupportKHR(index, surface, &ok));
			if(ok) has_present_queue = true;
		}
		index++;
	};
	if(!has_graphics_queue) return false;
	if(surface && !has_present_queue) return false;

	// check exts
	Set<std::string> extensions;
	enumateDeviceExts(gpu, extensions);
	for(auto& ext:exts) {
		if(ext.required) {
			if(!exists(extensions, ext.name)) 
				return false;
		}
	}
	return true;
}

std::vector<vk::DeviceQueueCreateInfo> Device::ChooseDeviceQueue(vk::SurfaceKHR surface) {
	std::vector<vk::DeviceQueueCreateInfo> queues;
	auto props = m_gpu.getQueueFamilyProperties();
	std::vector<uint32_t> graphic_indexs, present_indexs;
	uint32_t index = 0;
	for(auto& prop:props) {
		if(prop.queueFlags & vk::QueueFlagBits::eGraphics)
			graphic_indexs.push_back(index);
		index++;
	};
	m_graphics_queue.family_index = graphic_indexs.front();
	const static float defaultQueuePriority = 0.0f;
	{
		vk::DeviceQueueCreateInfo info;
		info
			.setQueueCount(1)
			.setQueueFamilyIndex(m_graphics_queue.family_index)
			.setPQueuePriorities(&defaultQueuePriority);
		queues.push_back(info);
	}
	m_present_queue.family_index = graphic_indexs.front();
	if(surface) {
		index = 0;
		for(auto& prop:props) {
			vk::Bool32 ok {false};
			VK_CHECK_RESULT(m_gpu.getSurfaceSupportKHR(index, surface, &ok))
			if(ok)
				present_indexs.push_back(index);
			index++;
		};
		if(present_indexs.empty()) {
			LOG_ERROR("not find present queue");
		} else if(graphic_indexs.front() != present_indexs.front()) {
			m_present_queue.family_index = present_indexs.front();
			vk::DeviceQueueCreateInfo info;
			info
				.setQueueCount(1)
				.setQueueFamilyIndex(m_present_queue.family_index)
				.setPQueuePriorities(&defaultQueuePriority);
			queues.push_back(info);
		}
	}
	return queues;
}

bool Device::Create(Instance& inst, Span<Extension> exts, vk::Extent2D extent, Device& device) {
	device.m_gpu = inst.gpu();
	device.m_limits = inst.gpu().getProperties().limits;
	device.set_out_extent(extent);

	Set<std::string> tested_exts;
	{
		enumateDeviceExts(inst.gpu(), device.m_extensions);
		for(auto& ext:exts) {
			bool ok = device.supportExt(ext.name);
			if(ok) tested_exts.insert(std::string(ext.name));
			if(ext.required && !ok) {
				LOG_ERROR("required vulkan device extension \"%s\" is not supported", ext.name.data());
				return false;
			}
		}
	}
	std::vector<std::string_view> tested_exts_c {tested_exts.begin(), tested_exts.end()};

	bool rq_surface = !inst.offscreen();
	VK_CHECK_RESULT_BOOL_RE(CreateDevice(inst, device.ChooseDeviceQueue(inst.surface()), tested_exts_c, &device.m_device));
	device.m_graphics_queue.handle = device.m_device.getQueue(device.m_graphics_queue.family_index, 0);
	device.m_present_queue.handle = device.m_device.getQueue(device.m_present_queue.family_index, 0);

	if(rq_surface) {
		if(!Swapchain::Create(device, inst.surface(), extent, device.m_swapchain)) {
			LOG_ERROR("create swapchain failed");
			return false;
		}
	}
	{
		vk::CommandPoolCreateInfo info;
		info.setFlags(vk::CommandPoolCreateFlagBits::eTransient |
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(device.m_graphics_queue.family_index);
		VK_CHECK_RESULT_ACT(return false, device.m_device.createCommandPool(&info, nullptr, &device.m_command_pool));
	}
	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.vulkanApiVersion = WP_VULKAN_VERSION;
		allocatorInfo.physicalDevice = device.m_gpu;
		allocatorInfo.device = device.m_device;
		allocatorInfo.instance = inst.inst();
		VK_CHECK_RESULT_ACT(return false, (vk::Result)vmaCreateAllocator(&allocatorInfo, &device.m_allocator));
	}
	device.m_tex_cache = std::make_unique<TextureCache>(device);
	return true;
}

vk::DeviceSize Device::GetUsage() const {
	VmaBudget budget;
	vmaGetHeapBudgets(m_allocator, &budget);
	return (vk::DeviceSize)budget.usage;
}

void Device::Destroy() {
	if(m_device) {
		VK_CHECK_RESULT(m_device.waitIdle());

		m_tex_cache->Destroy();
		m_device.destroyCommandPool(m_command_pool);

		for(auto& i:m_swapchain.images()) {
			m_device.destroyImageView(i.view);
		}
		if(m_swapchain.handle()) {
			m_device.destroySwapchainKHR(m_swapchain.handle());
		}
		vmaDestroyAllocator(m_allocator);
		m_device.destroy();
	}
}


void Device::DestroyBuffer(const BufferParameters& buf) const {
	if(buf.handle)
		vmaDestroyBuffer(m_allocator, buf.handle, buf.allocation);
}
void Device::DestroyImageParameters(const ImageParameters& image) const {
	m_device.destroySampler(image.sampler);
	m_device.destroyImageView(image.view);
	if(image.handle)
		vmaDestroyImage(m_allocator, image.handle, image.allocation);
}
void Device::DestroyExImageParameters(const ExImageParameters& image) const {
	m_device.destroySampler(image.sampler);
	m_device.destroyImageView(image.view);
	m_device.destroyImage(image.handle);
	m_device.freeMemory(image.mem);
}

void Device::DestroyPipeline(const PipelineParameters& pipe) const {
	m_device.destroyPipeline(pipe.handle);
	m_device.destroyPipelineLayout(pipe.layout);
	m_device.destroyRenderPass(pipe.pass);
	for(auto& d:pipe.descriptor_layouts)
		m_device.destroyDescriptorSetLayout(d);
}


Device::Device():m_tex_cache(std::make_unique<TextureCache>(*this)) {
}
Device::~Device() {};

bool Device::supportExt(std::string_view name) const {
	return exists(m_extensions, name);
}