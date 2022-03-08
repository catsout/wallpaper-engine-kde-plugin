#include "Instance.hpp"
#include "Device.hpp"

#include <cstdio>
#include "Utils/Logging.h"

using namespace wallpaper::vulkan;

#define ENABLE_VK_VALID_LAYER 1

constexpr std::array<const char *const, 1> required_layers = {
#if ENABLE_VK_VALID_LAYER
	"VK_LAYER_KHRONOS_validation"
#endif
};

constexpr std::array<const char *const, 0> device_exts = {
	//VK_KHR_SWAPCHAIN_EXTENSION_NAME

};


#define DECLARE_EXT_PFN(func) static PFN_##func pfn_##func;

DECLARE_EXT_PFN(vkCreateDebugUtilsMessengerEXT);
DECLARE_EXT_PFN(vkDestroyDebugUtilsMessengerEXT);
DECLARE_EXT_PFN(vkGetMemoryFdKHR);
DECLARE_EXT_PFN(vkCmdPushDescriptorSetKHR);

void GetExtProcFunc(vk::Instance* instance) {
	#define X(func) \
		pfn_##func = reinterpret_cast<PFN_##func>(instance->getProcAddr(#func));
	
	X(vkCreateDebugUtilsMessengerEXT);
	X(vkDestroyDebugUtilsMessengerEXT);
	X(vkGetMemoryFdKHR);
	X(vkCmdPushDescriptorSetKHR);
}

#define CALL_EXT_PFN(func, ...) \
    if (func != nullptr) { return func(__VA_ARGS__); } \
	else { return VK_ERROR_EXTENSION_NOT_PRESENT; }

#define CALL_EXT_PFN_VOID(func, ...) \
    if (func != nullptr) { func(__VA_ARGS__); }

VkResult vkCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger) {
	CALL_EXT_PFN(pfn_vkCreateDebugUtilsMessengerEXT, instance, pCreateInfo, pAllocator, pMessenger)
}

void vkDestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator) {
	CALL_EXT_PFN_VOID(pfn_vkDestroyDebugUtilsMessengerEXT, instance, messenger, pAllocator);
};

VkResult vkGetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd) {
	CALL_EXT_PFN(pfn_vkGetMemoryFdKHR, device, pGetFdInfo, pFd);
};

// Provided by VK_KHR_push_descriptor
void vkCmdPushDescriptorSetKHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites) {
	CALL_EXT_PFN_VOID(pfn_vkCmdPushDescriptorSetKHR, commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
}


static VkBool32 debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void                                       *pUserData) {
	VkBool32 result = VK_FALSE;	
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		result |= VK_TRUE;

		std::printf("validation layer: %s\n", pCallbackData->pMessage);
	}
	return result;
}

vk::Result setupDebugCallback(vk::Instance* instance, vk::DebugUtilsMessengerEXT& dcall) {
	vk::DebugUtilsMessengerCreateInfoEXT createInfo;
	createInfo
		.setFlags({})
		.setPNext(nullptr)
		.setMessageType(~vk::DebugUtilsMessageTypeFlagsEXT())
		.setMessageSeverity(~vk::DebugUtilsMessageSeverityFlagBitsEXT())
		.setPfnUserCallback(debugUtilsMessengerCallback)
		.setPUserData(nullptr);
	return instance->createDebugUtilsMessengerEXT(&createInfo, nullptr, &dcall);
}

static vk::Result CreatInstance(vk::Instance* inst, Span<const char*const> requiredExts) {
    vk::ApplicationInfo app_info;
	app_info
		.setPApplicationName("test_vulkan")
		.setPEngineName("test_vulkan")
		.setApplicationVersion(VK_API_VERSION_1_1)
		.setApiVersion(VK_API_VERSION_1_1)
		.setPNext(nullptr);


	std::vector<std::string> extension_names;
	std::vector<const char*> extension_names_c;
	{
		extension_names.push_back("VK_EXT_debug_utils");
		extension_names.insert(extension_names.end(), requiredExts.begin(), requiredExts.end());
		std::transform(extension_names.begin(), extension_names.end(), std::back_inserter(extension_names_c), [](const std::string& s){
			return s.c_str();
		});
	}

	vk::InstanceCreateInfo inst_info;
	inst_info
		.setPApplicationInfo(&app_info)
		.setEnabledExtensionCount(extension_names_c.size())
		.setPpEnabledExtensionNames(extension_names_c.data())
		.setEnabledLayerCount(required_layers.size())
		.setPpEnabledLayerNames(required_layers.data());

    vk::Result res = vk::createInstance(&inst_info, NULL, inst);
	if (res == vk::Result::eSuccess) {
		GetExtProcFunc(inst);
	}
	return res;
}

static bool ChoosePhysicalDevice(vk::Instance& instance, vk::PhysicalDevice& gpu, Span<std::uint8_t> uuid) {
	auto rv_deviceList = instance.enumeratePhysicalDevices();
	auto& deviceList = rv_deviceList.value;
	VkInstanceCreateInfo crea;
	auto logGpu = [](const vk::PhysicalDeviceProperties& props) {
		LOG_INFO("vulkan device: %s", props.deviceName.data());
	};
	// choose deiscrete device
	for(const auto& d:deviceList) {
		auto chains = d.getProperties2<vk::PhysicalDeviceProperties2,vk::PhysicalDeviceIDProperties>();
		auto& props = chains.get<vk::PhysicalDeviceProperties2>();
		if(uuid.size() > 0) {
			decltype(uuid) device_uuid {chains.get<vk::PhysicalDeviceIDProperties>().deviceUUID};
			if(uuid == device_uuid) {
				logGpu(props.properties);
				for(const auto& p:device_uuid) {
					//printf("%02x ", p);
				}
				gpu = d;
				return true;
			}
		} else { 
			if (props.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
				logGpu(props.properties);
				gpu = d;
				return true;
			}
		}
	}
	return false;
}


const vk::Instance& Instance::inst() const { return m_inst; };
const vk::PhysicalDevice& Instance::gpu() const { return m_gpu; }
const vk::SurfaceKHR& Instance::surface() const { return m_surface; }

bool Instance::offscreen() const { return ! m_surface; }

void Instance::setSurface(vk::SurfaceKHR sf) {
	m_surface = sf;
}

void Instance::Destroy() {
	if(m_inst) {
		if(m_surface) {
			m_inst.destroySurfaceKHR(m_surface);
			LOG_INFO("Destory surface");
		}
		m_inst.destroyDebugUtilsMessengerEXT(m_debug_utils);
		m_inst.destroy();
		LOG_INFO("Destory instance");
	}
}

bool Instance::Create(Instance& inst, Span<const char*const> instanceExts, Span<std::uint8_t> uuid) {
	VK_CHECK_RESULT_ACT(return false, CreatInstance(&inst.m_inst, instanceExts));
	VK_CHECK_RESULT_ACT(return false, setupDebugCallback(&inst.m_inst, inst.m_debug_utils));
	if(!ChoosePhysicalDevice(inst.m_inst, inst.m_gpu, uuid)) {
		if(uuid.size() > 0) {
			// to do
		}
		return false;
	}
	return true;
}