#include "Instance.hpp"
#include "Device.hpp"

#include <cstdio>
#include "Utils/Logging.h"

using namespace wallpaper::vulkan;

#define ENABLE_VK_VALID_LAYER 1


constexpr std::array<InstanceLayer, 0> base_inst_layers {
};

constexpr std::array base_inst_exts {
    Extension { true, VK_EXT_DEBUG_UTILS_EXTENSION_NAME }
};

#define DECLARE_EXT_PFN(func) static PFN_##func pfn_##func;

DECLARE_EXT_PFN(vkCreateDebugUtilsMessengerEXT);
DECLARE_EXT_PFN(vkDestroyDebugUtilsMessengerEXT);
DECLARE_EXT_PFN(vkGetMemoryFdKHR);
DECLARE_EXT_PFN(vkGetSemaphoreFdKHR);
DECLARE_EXT_PFN(vkCmdPushDescriptorSetKHR);

void GetExtProcFunc(vk::Instance* instance) {
	#define X(func) \
		pfn_##func = reinterpret_cast<PFN_##func>(instance->getProcAddr(#func));
	
	X(vkCreateDebugUtilsMessengerEXT);
	X(vkDestroyDebugUtilsMessengerEXT);
	X(vkGetMemoryFdKHR);
	X(vkGetSemaphoreFdKHR);
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

VkResult vkGetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd) {
	CALL_EXT_PFN(pfn_vkGetSemaphoreFdKHR, device, pGetFdInfo, pFd);
}

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

static vk::Result CreatInstance(vk::Instance* inst, Span<std::string_view> exts, Span<std::string_view> layers) {
    vk::ApplicationInfo app_info;
	app_info
		.setPApplicationName(WP_APPLICATION_NAME)
		.setPEngineName("vulkan")
		.setApplicationVersion(WP_VULKAN_VERSION)
		.setApiVersion(WP_VULKAN_VERSION)
		.setPNext(nullptr);


	std::vector<const char*> extension_names_c;
	std::transform(exts.begin(), exts.end(), std::back_inserter(extension_names_c), [](auto& ext){
		return ext.data();
	});

	std::vector<const char*> layer_names_c;
	std::transform(layers.begin(), layers.end(), std::back_inserter(layer_names_c), [](auto& layer){
		return layer.data();
	});

	vk::InstanceCreateInfo inst_info;
	inst_info
		.setPApplicationInfo(&app_info)
		.setEnabledExtensionCount(extension_names_c.size())
		.setPpEnabledExtensionNames(extension_names_c.data())
		.setEnabledLayerCount(layer_names_c.size())
		.setPpEnabledLayerNames(layer_names_c.data());

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

static void enumateExts(wallpaper::Set<std::string>& set) {
	auto rv = vk::enumerateInstanceExtensionProperties();
	VK_CHECK_RESULT_VOID_RE(rv.result);
	for(auto& ext:rv.value) {
		set.insert(ext.extensionName);
	}
}
static void enumateLayers(wallpaper::Set<std::string>& set) {
	auto rv = vk::enumerateInstanceLayerProperties();
	VK_CHECK_RESULT_VOID_RE(rv.result);
	for(auto& ext:rv.value) {
		set.insert(ext.layerName);
	}
}


const vk::Instance& Instance::inst() const { return m_inst; };
const vk::PhysicalDevice& Instance::gpu() const { return m_gpu; }
const vk::SurfaceKHR& Instance::surface() const { return m_surface; }

bool Instance::offscreen() const { return ! m_surface; }

void Instance::setSurface(vk::SurfaceKHR sf) {
	m_surface = sf;
}

bool Instance::supportExt(std::string_view name) const {
	return exists(m_extensions, name);
}
bool Instance::supportLayer(std::string_view name) const {
	return exists(m_layers, name);
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

bool Instance::Create(Instance& inst, Span<Extension> instExts, Span<InstanceLayer> instLayers, Span<std::uint8_t> uuid) {
	enumateExts(inst.m_extensions);
	Set<std::string> exts, layers;
	std::array test_exts_array { Span<Extension>(base_inst_exts), instExts };	
	for(auto& test_exts:test_exts_array) {
		for(auto& ext:test_exts) {
			bool ok = inst.supportExt(ext.name);
			if(ok) exts.insert(std::string(ext.name));
			if(ext.required && !ok) {
				LOG_ERROR("required vulkan instance extension \"%s\" is not supported", ext.name.data());
				return false;
			}
		}
	}

	enumateLayers(inst.m_layers);
	std::array test_layers_array { Span<InstanceLayer>(base_inst_layers), instLayers };	
	for(auto& test_layers:test_layers_array) {
		for(auto& layer:test_layers) {
			bool ok = inst.supportLayer(layer.name);
			if(ok) layers.insert(std::string(layer.name));
			if(layer.required && !ok) {
				LOG_ERROR("required vulkan instance layer \"%s\" is not supported", layer.name.data());
				return false;
			}
		}
	}

	std::vector<std::string_view> exts_vec{exts.begin(), exts.end()}, layers_vec{layers.begin(), layers.end()};
	VK_CHECK_RESULT_ACT(return false, CreatInstance(&inst.m_inst, exts_vec, layers_vec));
	VK_CHECK_RESULT_ACT(return false, setupDebugCallback(&inst.m_inst, inst.m_debug_utils));

	if(!ChoosePhysicalDevice(inst.m_inst, inst.m_gpu, uuid)) {
		if(uuid.size() > 0) {
			// to do
		}
		return false;
	}
	return true;
}