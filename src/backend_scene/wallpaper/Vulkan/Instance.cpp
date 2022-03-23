#include "Instance.hpp"
#include "Device.hpp"

#include <cstdio>
#include "Utils/Logging.h"

using namespace wallpaper::vulkan;

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE


constexpr std::array<InstanceLayer, 0> base_inst_layers {
};

constexpr std::array base_inst_exts {
    Extension { true, VK_EXT_DEBUG_UTILS_EXTENSION_NAME }
};


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

static void LoadBasicVkFunc() {
	vk::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
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
		//GetExtProcFunc(inst);
		VULKAN_HPP_DEFAULT_DISPATCHER.init(*inst);
	}
	return res;
}

bool Instance::ChoosePhysicalDevice(const CheckGpuOp& checkgpu, Span<std::uint8_t> uuid) {
	auto rv_deviceList = m_inst.enumeratePhysicalDevices();
	VK_CHECK_RESULT_BOOL_RE(rv_deviceList.result);
	auto& deviceList = rv_deviceList.value;
	VkInstanceCreateInfo crea;
	auto logGpu = [](const vk::PhysicalDeviceProperties& props) {
		LOG_INFO("vulkan device: %s", (const char*)props.deviceName);
	};

	vk::PhysicalDevice final_gpu;
	vk::PhysicalDeviceProperties final_props;
	// choose deiscrete device
	for(const auto& d:deviceList) {
		auto chains = d.getProperties2<vk::PhysicalDeviceProperties2,vk::PhysicalDeviceIDProperties>();
		auto& props = chains.get<vk::PhysicalDeviceProperties2>();
		if(uuid.size() > 0) {
			decltype(uuid) device_uuid {chains.get<vk::PhysicalDeviceIDProperties>().deviceUUID};
			if(uuid == device_uuid) {
				/*
				for(const auto& p:device_uuid) {
					printf("%02x ", p);
				}
				*/
				final_props = props.properties;
				final_gpu = d;
				break;
			}
		} else { 
			if(checkgpu(d)) {
				final_props = props.properties;
				final_gpu = d;
				break;
			}
		}
	}
	if(final_gpu) {
		logGpu(final_props);
		m_gpu = final_gpu;
		return true;
	} else {
		LOG_ERROR("failed to find GPU with vulkan support");
		return false;
	}
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

bool Instance::Create(Instance& inst, Span<Extension> instExts, Span<InstanceLayer> instLayers) {
	LoadBasicVkFunc();	

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

	/*
	if(!ChoosePhysicalDevice(inst.m_inst, inst.m_gpu, checkgpu, uuid)) {
		if(uuid.size() > 0) {
			// to do
		}
		return false;
	}
	*/
	return true;
}