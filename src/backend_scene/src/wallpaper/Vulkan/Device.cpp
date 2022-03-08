#include "Device.hpp"

#include "Utils/Logging.h"
#include "GraphicsPipeline.hpp"

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
			vk::Bool32 ok;
			VK_CHECK_RESULT(m_gpu.getSurfaceSupportKHR(index, surface, &ok))
			if(ok)
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

bool Device::Create(Instance& inst, Span<const char*const> exts, Device& device) {
	vk::Result result {vk::Result::eIncomplete};
	device.m_gpu = inst.gpu();
	device.m_limits = inst.gpu().getProperties().limits;

	bool rq_surface = !inst.offscreen();
	VK_CHECK_RESULT_BOOL_RE(CreateDevice(inst, device.ChooseDeviceQueue(inst.surface()), exts, &device.m_device));
	device.m_graphics_queue.handle = device.m_device.getQueue(device.m_graphics_queue.family_index, 0);
	device.m_present_queue.handle = device.m_device.getQueue(device.m_present_queue.family_index, 0);

	if(rq_surface) {
		if(!Swapchain::Create(device, inst.surface(), {1280, 720}, device.m_swapchain)) {
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
		m_device.destroySwapchainKHR(m_swapchain.handle());
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