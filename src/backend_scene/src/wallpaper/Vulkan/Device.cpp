#include "Device.hpp"

#include "Utils/Logging.h"

using namespace wallpaper::vulkan;

vk::Result CreateDevice(Instance& instance,
	Span<vk::DeviceQueueCreateInfo> queueCreateInfos,
	Span<const char*const> deviceExts,
	vk::Device* device) {
	
	vk::DeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo
		.setQueueCreateInfoCount(queueCreateInfos.size())
		.setPQueueCreateInfos(queueCreateInfos.data())
		.setEnabledExtensionCount(deviceExts.size())
		.setPpEnabledExtensionNames(deviceExts.data());

    return instance.gpu().createDevice(&deviceCreateInfo, nullptr, device); 
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
	if(surface) {
		index = 0;
		for(auto& prop:props) {
			if(m_gpu.getSurfaceSupportKHR(index, surface).value)
				present_indexs.push_back(index);
			index++;
		};
		m_present_queue.family_index = graphic_indexs.front();
		if(graphic_indexs.front() != present_indexs.front()) {
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

vk::Result Device::Create(Instance& inst, Span<const char*const> exts, Device& device) {
	vk::Result result {vk::Result::eIncomplete};
	device.m_gpu = inst.gpu();
	device.m_limits = inst.gpu().getProperties().limits;

	bool rq_surface = !inst.offscreen();
	do {
		result = CreateDevice(inst, device.ChooseDeviceQueue(inst.surface()), exts, &device.m_device);
		if(result != vk::Result::eSuccess) break;
		device.m_graphics_queue.handle = device.m_device.getQueue(device.m_graphics_queue.family_index, 0);
		device.m_present_queue.handle = device.m_device.getQueue(device.m_present_queue.family_index, 0);

		if(rq_surface) {
			result = Swapchain::Create(device, inst.surface(), {1280, 720}, device.m_swapchain);
			if(result != vk::Result::eSuccess) {
				LOG_ERROR("create swapchain failed");
				break;
			}
		}

	  	auto rv_cmdpool = device.m_device.createCommandPool({
			vk::CommandPoolCreateFlagBits::eTransient|
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			device.m_graphics_queue.family_index});
		if(rv_cmdpool.result != vk::Result::eSuccess) { result = rv_cmdpool.result; break; }
		device.m_command_pool = rv_cmdpool.value;
		{
			VmaAllocatorCreateInfo allocatorInfo = {};
			allocatorInfo.physicalDevice = device.m_gpu;
			allocatorInfo.device = device.m_device;
			allocatorInfo.instance = inst.inst();
			vmaCreateAllocator(&allocatorInfo, &device.m_allocator);
		}
		device.m_tex_cache = std::make_unique<TextureCache>(device);
		result = vk::Result::eSuccess;
	} while(false);
	return result;
}

void Device::Destroy() {
	if(m_device) {
		(void)m_device.waitIdle();
		m_tex_cache->Destroy();
		m_device.destroyCommandPool(m_command_pool);
		vmaDestroyAllocator(m_allocator);
		m_device.destroy();
	}
}


void Device::DestroyBuffer(const BufferParameters& buf) const {
	if(buf.handle) {
		vmaDestroyBuffer(m_allocator, buf.handle, buf.allocation);
	}
}


Device::Device():m_tex_cache(std::make_unique<TextureCache>(*this)) {
}
Device::~Device() {};

/*
Device::Device(Device&& o) {
	*this = std::move(o);
}
Device& Device::operator=(Device&& o) {
	m_device = o.m_device;
	m_gpu = o.m_gpu;
	m_swapchain = o.m_swapchain;
	m_allocator = o.m_allocator;
	m_command_pool = o.m_command_pool;
	m_graphics_queue = o.m_graphics_queue;
	m_present_queue = o.m_present_queue;
	m_tex_cache = std::move(o.m_tex_cache);
	return *this;
}
*/