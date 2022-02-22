#include "Instance.hpp"
#include "Device.hpp"

#include <cstdio>
#include "Utils/Logging.h"

using namespace wallpaper::vulkan;

#define ENABLE_VK_VALID_LAYER 1

constexpr std::array required_layers = {
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

void GetExtProcFunc(vk::Instance* instance) {
	#define X(func) \
		pfn_##func = reinterpret_cast<PFN_##func>(instance->getProcAddr(#func));
	
	X(vkCreateDebugUtilsMessengerEXT);
	X(vkDestroyDebugUtilsMessengerEXT);
	X(vkGetMemoryFdKHR);
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


static VkBool32 debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void                                       *pUserData) {
	VkBool32 result = VK_FALSE;	
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		result |= VK_TRUE;

		std::printf("validation layer: %s", pCallbackData->pMessage);
	}
	return result;
}

vk::DebugUtilsMessengerEXT setupDebugCallback(vk::Instance* instance) {
	vk::DebugUtilsMessengerCreateInfoEXT createInfo;
	createInfo
		.setFlags({})
		.setPNext(nullptr)
		.setMessageType(~vk::DebugUtilsMessageTypeFlagsEXT())
		.setMessageSeverity(~vk::DebugUtilsMessageSeverityFlagBitsEXT())
		.setPfnUserCallback(debugUtilsMessengerCallback)
		.setPUserData(nullptr);
	return instance->createDebugUtilsMessengerEXT(createInfo);
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

void Instance::setSurface(vk::SurfaceKHR sf) {
	m_surface = sf;
}

void Instance::Destroy() {
	if(m_inst) {
		m_inst.destroyDebugUtilsMessengerEXT(m_debug_utils);
		m_inst.destroy();
	}
}

vk::ResultValue<Instance> Instance::Create(Span<const char*const> instanceExts, Span<std::uint8_t> uuid) {
	vk::ResultValue<Instance> rv {vk::Result::eIncomplete, {}};
	do {
		Instance& inst = rv.value;
		rv.result = CreatInstance(&inst.m_inst, instanceExts);
		if(rv.result != vk::Result::eSuccess) break;
		else rv.result = vk::Result::eIncomplete;
		inst.m_debug_utils = setupDebugCallback(&inst.m_inst);
		if(!ChoosePhysicalDevice(inst.m_inst, inst.m_gpu, uuid)) break;
		rv.result = vk::Result::eSuccess;
	} while(false);
	return rv;
}