#include "vvk/vulkan_wrapper.hpp"

namespace
{

}

namespace vvk
{
template<typename T>
bool Proc(T& result, const InstanceDispatch& dld, const char* proc_name,
          VkInstance instance = nullptr) noexcept {
    result = reinterpret_cast<T>(dld.vkGetInstanceProcAddr(instance, proc_name));
    return result != nullptr;
}

template<typename T>
void Proc(T& result, const DeviceDispatch& dld, const char* proc_name, VkDevice device) noexcept {
    result = reinterpret_cast<T>(dld.vkGetDeviceProcAddr(device, proc_name));
}

bool Load(InstanceDispatch& dld) noexcept {
#define X(name) Proc(dld.name, dld, #name)
    return X(vkCreateInstance) && X(vkEnumerateInstanceExtensionProperties) &&
           X(vkEnumerateInstanceLayerProperties);
#undef X
}

VkResult LoadLibrary(utils::DynamicLibrary& dlib, vvk::InstanceDispatch& dld) {
    using namespace utils;
    dlib = DynamicLibrary("libvulkan.so.1");
    if (! dlib.IsOpen()) dlib = DynamicLibrary("libvulkan.so");
    if (! dlib.IsOpen()) return VK_ERROR_INITIALIZATION_FAILED;

    if (! dlib.GetSymbol("vkGetInstanceProcAddr", dld.vkGetInstanceProcAddr))
        return VK_ERROR_INITIALIZATION_FAILED;
    return VK_SUCCESS;
}

bool Load(VkInstance instance, InstanceDispatch& dld) noexcept {
#define X(name) Proc(dld.name, dld, #name, instance)
    X(vkCreateDebugUtilsMessengerEXT);
    X(vkDestroyDebugUtilsMessengerEXT);
    X(vkDestroySurfaceKHR);
    X(vkGetPhysicalDeviceFeatures2KHR);
    X(vkGetPhysicalDeviceProperties2KHR);
    X(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    X(vkGetPhysicalDeviceSurfaceFormatsKHR);
    X(vkGetPhysicalDeviceSurfacePresentModesKHR);
    X(vkGetPhysicalDeviceSurfaceSupportKHR);
    X(vkGetSwapchainImagesKHR);
    X(vkQueuePresentKHR);

    return X(vkDestroyInstance) && X(vkCreateDevice) && X(vkDestroyDevice) && X(vkDestroyDevice) &&
           X(vkEnumerateDeviceExtensionProperties) && X(vkEnumeratePhysicalDevices) &&
           X(vkGetDeviceProcAddr) && X(vkGetPhysicalDeviceFormatProperties) &&
           X(vkGetPhysicalDeviceMemoryProperties) && X(vkGetPhysicalDeviceMemoryProperties2) &&
           X(vkGetPhysicalDeviceProperties) && X(vkGetPhysicalDeviceQueueFamilyProperties);
#undef X
}

bool Load(VkDevice device, DeviceDispatch& dld) noexcept {
#define X(name) Proc(dld.name, dld, #name, device)
    X(vkAcquireNextImageKHR);
    X(vkAllocateCommandBuffers);
    X(vkAllocateDescriptorSets);
    X(vkAllocateMemory);
    X(vkBeginCommandBuffer);
    X(vkBindBufferMemory);
    X(vkBindImageMemory);
    X(vkCmdBeginQuery);
    X(vkCmdBeginRenderPass);
    X(vkCmdBindDescriptorSets);
    X(vkCmdBindIndexBuffer);
    X(vkCmdBindPipeline);
    X(vkCmdBindVertexBuffers);
    X(vkCmdBlitImage);
    X(vkCmdClearColorImage);
    X(vkCmdClearAttachments);
    X(vkCmdCopyBuffer);
    X(vkCmdCopyBufferToImage);
    X(vkCmdCopyImage);
    X(vkCmdCopyImageToBuffer);
    X(vkCmdDispatch);
    X(vkCmdDraw);
    X(vkCmdDrawIndexed);
    X(vkCmdEndQuery);
    X(vkCmdEndRenderPass);
    X(vkCmdEndDebugUtilsLabelEXT);
    X(vkCmdFillBuffer);
    X(vkCmdPipelineBarrier);
    X(vkCmdPushConstants);
    X(vkCmdPushDescriptorSetKHR);
    X(vkCmdPushDescriptorSetWithTemplateKHR);
    X(vkCmdSetBlendConstants);
    X(vkCmdSetDepthBias);
    X(vkCmdSetDepthBounds);
    X(vkCmdSetEvent);
    X(vkCmdSetScissor);
    X(vkCmdSetStencilCompareMask);
    X(vkCmdSetStencilReference);
    X(vkCmdSetStencilWriteMask);
    X(vkCmdSetViewport);
    X(vkCmdWaitEvents);
    X(vkCmdSetLineWidth);
    X(vkCmdResolveImage);
    X(vkCreateBuffer);
    X(vkCreateBufferView);
    X(vkCreateCommandPool);
    X(vkCreateComputePipelines);
    X(vkCreateDescriptorPool);
    X(vkCreateDescriptorSetLayout);
    X(vkCreateDescriptorUpdateTemplateKHR);
    X(vkCreateEvent);
    X(vkCreateFence);
    X(vkCreateFramebuffer);
    X(vkCreateGraphicsPipelines);
    X(vkCreateImage);
    X(vkCreateImageView);
    X(vkCreatePipelineLayout);
    X(vkCreateQueryPool);
    X(vkCreateRenderPass);
    X(vkCreateSampler);
    X(vkCreateSemaphore);
    X(vkCreateShaderModule);
    X(vkCreateSwapchainKHR);
    X(vkDestroyBuffer);
    X(vkDestroyBufferView);
    X(vkDestroyCommandPool);
    X(vkDestroyDescriptorPool);
    X(vkDestroyDescriptorSetLayout);
    X(vkDestroyDescriptorUpdateTemplateKHR);
    X(vkDestroyEvent);
    X(vkDestroyFence);
    X(vkDestroyFramebuffer);
    X(vkDestroyImage);
    X(vkDestroyImageView);
    X(vkDestroyPipeline);
    X(vkDestroyPipelineLayout);
    X(vkDestroyQueryPool);
    X(vkDestroyRenderPass);
    X(vkDestroySampler);
    X(vkDestroySemaphore);
    X(vkDestroyShaderModule);
    X(vkDestroySwapchainKHR);
    X(vkDeviceWaitIdle);
    X(vkEndCommandBuffer);
    X(vkFreeCommandBuffers);
    X(vkFreeDescriptorSets);
    X(vkFreeMemory);
    X(vkGetBufferMemoryRequirements2);
    X(vkGetDeviceQueue);
    X(vkGetEventStatus);
    X(vkGetFenceStatus);
    X(vkGetImageMemoryRequirements);
    X(vkGetMemoryFdKHR);
    X(vkGetQueryPoolResults);
    X(vkGetPipelineExecutablePropertiesKHR);
    X(vkGetPipelineExecutableStatisticsKHR);
    X(vkGetSemaphoreCounterValueKHR);
    X(vkMapMemory);
    X(vkQueueSubmit);
    X(vkResetFences);
    X(vkSetDebugUtilsObjectNameEXT);
    X(vkSetDebugUtilsObjectTagEXT);
    X(vkUnmapMemory);
    X(vkUpdateDescriptorSetWithTemplateKHR);
    X(vkUpdateDescriptorSets);
    X(vkWaitForFences);
    X(vkWaitSemaphoresKHR);
#undef X
    return true;
}

void Destroy(VkInstance instance, const InstanceDispatch& dld) noexcept {
    dld.vkDestroyInstance(instance, nullptr);
}

void Destroy(VkDevice device, const InstanceDispatch& dld) noexcept {
    dld.vkDestroyDevice(device, nullptr);
}

void Destroy(VkInstance Instance, VkDebugUtilsMessengerEXT handle,
             const InstanceDispatch& dld) noexcept {
    dld.vkDestroyDebugUtilsMessengerEXT(Instance, handle, nullptr);
}

void Destroy(VkDevice device, VkCommandPool handle, const DeviceDispatch& dld) noexcept {
    dld.vkDestroyCommandPool(device, handle, nullptr);
}
void Destroy(VkDevice device, VkPipeline handle, const DeviceDispatch& dld) noexcept {
    dld.vkDestroyPipeline(device, handle, nullptr);
}
void Destroy(VkDevice device, VkPipelineLayout handle, const DeviceDispatch& dld) noexcept {
    dld.vkDestroyPipelineLayout(device, handle, nullptr);
}

void Destroy(VkDevice device, VkRenderPass handle, const DeviceDispatch& dld) noexcept {
    dld.vkDestroyRenderPass(device, handle, nullptr);
}

void Destroy(VkDevice device, VkDescriptorSetLayout handle, const DeviceDispatch& dld) noexcept {
    dld.vkDestroyDescriptorSetLayout(device, handle, nullptr);
}

void Destroy(VkDevice device, VkImage handle, const DeviceDispatch& dld) noexcept {
    dld.vkDestroyImage(device, handle, nullptr);
}

void Destroy(VkDevice device, VkImageView handle, const DeviceDispatch& dld) noexcept {
    dld.vkDestroyImageView(device, handle, nullptr);
}

void Destroy(VkDevice device, VkDeviceMemory handle, const DeviceDispatch& dld) noexcept {
    dld.vkFreeMemory(device, handle, nullptr);
}

void Destroy(VkDevice device, VkShaderModule handle, const DeviceDispatch& dld) noexcept {
    dld.vkDestroyShaderModule(device, handle, nullptr);
}

void Destroy(VkDevice device, VkSwapchainKHR handle, const DeviceDispatch& dld) noexcept {
    dld.vkDestroySwapchainKHR(device, handle, nullptr);
}

void Destroy(VkDevice device, VkSampler handle, const DeviceDispatch& dld) noexcept {
    dld.vkDestroySampler(device, handle, nullptr);
}

void Destroy(VkDevice device, VkSemaphore handle, const DeviceDispatch& dld) noexcept {
    dld.vkDestroySemaphore(device, handle, nullptr);
}

void Destroy(VkDevice device, VkFence handle, const DeviceDispatch& dld) noexcept {
    dld.vkDestroyFence(device, handle, nullptr);
}

void Destroy(VkDevice device, VkFramebuffer handle, const DeviceDispatch& dld) noexcept {
    dld.vkDestroyFramebuffer(device, handle, nullptr);
}

void Destroy(VkInstance instance, VkSurfaceKHR handle, const InstanceDispatch& dld) noexcept {
    dld.vkDestroySurfaceKHR(instance, handle, nullptr);
}

VkResult Free(VkDevice device, VkCommandPool pool, Span<VkCommandBuffer> allos,
              const DeviceDispatch& dld) noexcept {
    dld.vkFreeCommandBuffers(device, pool, (uint32_t)allos.size(), allos.data());
    return VK_SUCCESS;
}

VkResult Instance::Create(Instance& inst, const VkApplicationInfo& app_info,
                          Span<const char*> layers, Span<const char*> extensions,
                          InstanceDispatch& dld) noexcept {
    VkInstanceCreateInfo ci {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = 0,
        .pApplicationInfo        = &app_info,
        .enabledLayerCount       = layers.size(),
        .ppEnabledLayerNames     = layers.data(),
        .enabledExtensionCount   = extensions.size(),
        .ppEnabledExtensionNames = extensions.data(),
    };

    VkInstance instance;
    VkResult   res = dld.vkCreateInstance(&ci, nullptr, &instance);
    if (res == VK_SUCCESS) {
        if (Proc(dld.vkDestroyInstance, dld, "vkDestroyInstance", instance))
            inst = Instance(instance, dld);
        else
            res = VK_ERROR_INITIALIZATION_FAILED;
    }
    return res;
}

std::vector<PhysicalDevice> Instance::EnumeratePhysicalDevices() const noexcept {
    uint32_t num;
    VVK_CHECK(dld->vkEnumeratePhysicalDevices(handle, &num, nullptr));
    std::vector<VkPhysicalDevice> vkphysical_devices(num);
    std::vector<PhysicalDevice>   physical_devices(num);
    VVK_CHECK(dld->vkEnumeratePhysicalDevices(handle, &num, vkphysical_devices.data()));
    std::transform(vkphysical_devices.begin(),
                   vkphysical_devices.end(),
                   physical_devices.begin(),
                   [this](const VkPhysicalDevice& vkphy) {
                       return PhysicalDevice(vkphy, *dld);
                   });
    return physical_devices;
}

DebugUtilsMessenger Instance::CreateDebugUtilsMessenger(
    const VkDebugUtilsMessengerCreateInfoEXT& create_info) const noexcept {
    VkDebugUtilsMessengerEXT object;
    VVK_CHECK(dld->vkCreateDebugUtilsMessengerEXT(handle, &create_info, nullptr, &object));
    return DebugUtilsMessenger(object, handle, *dld);
}
VkResult Device::Create(Device& device, VkPhysicalDevice physical_device,
                        Span<const VkDeviceQueueCreateInfo> queues_ci,
                        Span<const char*> enabled_extensions, const void* next,
                        DeviceDispatch& dld) {
    const VkDeviceCreateInfo ci {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = next,
        .flags                   = 0,
        .queueCreateInfoCount    = queues_ci.size(),
        .pQueueCreateInfos       = queues_ci.data(),
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = nullptr,
        .enabledExtensionCount   = enabled_extensions.size(),
        .ppEnabledExtensionNames = enabled_extensions.data(),
        .pEnabledFeatures        = nullptr,
    };
    VkDevice vkdevice;
    VkResult res = dld.vkCreateDevice(physical_device, &ci, nullptr, &vkdevice);
    if (res == VK_SUCCESS) {
        Load(vkdevice, dld);
        device = Device(vkdevice, dld);
    }
    return res;
}

Queue Device::GetQueue(uint32_t family_index) const noexcept {
    VkQueue queue;
    dld->vkGetDeviceQueue(handle, family_index, 0, &queue);
    return Queue(queue, *dld);
}

VkMemoryRequirements Device::GetImageMemoryRequirements(VkImage image) const noexcept {
    VkMemoryRequirements requirements;
    dld->vkGetImageMemoryRequirements(handle, image, &requirements);
    return requirements;
}

VkResult Device::AllocateMemory(const VkMemoryAllocateInfo& ai, DeviceMemory& mem) const noexcept {
    VkDeviceMemory memory;
    auto           res = dld->vkAllocateMemory(handle, &ai, nullptr, &memory);
    if (res == VK_SUCCESS) mem = DeviceMemory(memory, handle, *dld);
    return res;
}

VkResult Device::CreateCommandPool(const VkCommandPoolCreateInfo& ci, CommandPool& pool) const {
    VkCommandPool vkpool;
    VkResult      res = dld->vkCreateCommandPool(handle, &ci, nullptr, &vkpool);
    if (res == VK_SUCCESS) pool = CommandPool(vkpool, handle, *dld);
    return res;
}

VkResult Device::CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& ci,
                                           DescriptorSetLayout& layout) const noexcept {
    VkDescriptorSetLayout object;
    VkResult              res = dld->vkCreateDescriptorSetLayout(handle, &ci, nullptr, &object);
    if (res == VK_SUCCESS) layout = DescriptorSetLayout(object, handle, *dld);
    return res;
}

VkResult Device::CreateRenderPass(const VkRenderPassCreateInfo& ci,
                                  RenderPass&                   pass) const noexcept {
    VkRenderPass object;
    VkResult     res = dld->vkCreateRenderPass(handle, &ci, nullptr, &object);
    if (res == VK_SUCCESS) pass = RenderPass(object, handle, *dld);
    return res;
}

VkResult Device::CreatePipelineLayout(const VkPipelineLayoutCreateInfo& ci,
                                      PipelineLayout&                   layout) const noexcept {
    VkPipelineLayout object;
    VkResult         res = dld->vkCreatePipelineLayout(handle, &ci, nullptr, &object);
    if (res == VK_SUCCESS) layout = PipelineLayout(object, handle, *dld);
    return res;
}

VkResult Device::CreateSwapchainKHR(const VkSwapchainCreateInfoKHR& ci,
                                    SwapchainKHR&                   surface) const noexcept {
    VkSwapchainKHR object;
    VkResult       res = dld->vkCreateSwapchainKHR(handle, &ci, nullptr, &object);
    if (res == VK_SUCCESS) surface = SwapchainKHR(object, handle, *dld);
    return res;
}

VkResult Device::CreateShaderModule(const VkShaderModuleCreateInfo& ci,
                                    ShaderModule&                   sm) const noexcept {
    VkShaderModule object;
    VkResult       res = dld->vkCreateShaderModule(handle, &ci, nullptr, &object);
    if (res == VK_SUCCESS) sm = ShaderModule(object, handle, *dld);
    return res;
}

VkResult Device::CreateSemaphore(const VkSemaphoreCreateInfo& ci, Semaphore& sm) const noexcept {
    VkSemaphore object;
    VkResult    res = dld->vkCreateSemaphore(handle, &ci, nullptr, &object);
    if (res == VK_SUCCESS) sm = Semaphore(object, handle, *dld);
    return res;
}

VkResult Device::CreateImage(const VkImageCreateInfo& ci, Image& img) const noexcept {
    VkImage object;

    VkResult res = dld->vkCreateImage(handle, &ci, nullptr, &object);
    if (res == VK_SUCCESS) img = Image(object, handle, *dld);
    return res;
}

VkResult Device::CreateImageView(const VkImageViewCreateInfo& ci, ImageView& view) const noexcept {
    VkImageView object;
    VkResult    res = dld->vkCreateImageView(handle, &ci, nullptr, &object);
    if (res == VK_SUCCESS) view = ImageView(object, handle, *dld);
    return res;
}

VkResult Device::CreateFramebuffer(const VkFramebufferCreateInfo& ci,
                                   Framebuffer&                   fb) const noexcept {
    VkFramebuffer object;
    VkResult      res = dld->vkCreateFramebuffer(handle, &ci, nullptr, &object);
    if (res == VK_SUCCESS) fb = Framebuffer(object, handle, *dld);
    return res;
}

VkResult Device::CreateFence(const VkFenceCreateInfo& ci, Fence& fe) const noexcept {
    VkFence  object;
    VkResult res = dld->vkCreateFence(handle, &ci, nullptr, &object);
    if (res == VK_SUCCESS) fe = Fence(object, handle, *dld);
    return res;
}

VkResult Device::CreateSampler(const VkSamplerCreateInfo& ci, Sampler& sam) const noexcept {
    VkSampler object;
    VkResult  res = dld->vkCreateSampler(handle, &ci, nullptr, &object);
    if (res == VK_SUCCESS) sam = Sampler(object, handle, *dld);
    return res;
}

VkResult Device::CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& ci,
                                        Pipeline& pipeline) const noexcept {
    VkPipeline object;
    VkResult res = dld->vkCreateGraphicsPipelines(handle, VK_NULL_HANDLE, 1, &ci, nullptr, &object);
    if (res == VK_SUCCESS) pipeline = Pipeline(object, handle, *dld);
    return res;
}

VkResult Buffer::BindMemory(VkDeviceMemory memory, VkDeviceSize offset) const noexcept {
    return dld->vkBindBufferMemory(owner, handle, memory, offset);
}

VkResult Image::BindMemory(VkDeviceMemory memory, VkDeviceSize offset) const noexcept {
    return dld->vkBindImageMemory(owner, handle, memory, offset);
}

VkResult SwapchainKHR::GetImages(std::vector<VkImage>& images) const {
    uint32_t num;
    if (auto res = dld->vkGetSwapchainImagesKHR(owner, handle, &num, nullptr); res != VK_SUCCESS)
        return res;
    images.resize(num);
    return dld->vkGetSwapchainImagesKHR(owner, handle, &num, images.data());
}

VkPhysicalDeviceProperties PhysicalDevice::GetProperties() const noexcept {
    VkPhysicalDeviceProperties props;
    dld->vkGetPhysicalDeviceProperties(handle, &props);
    return props;
}

void PhysicalDevice::GetProperties2KHR(VkPhysicalDeviceProperties2KHR& props) const noexcept {
    dld->vkGetPhysicalDeviceProperties2KHR(handle, &props);
}

VkResult PhysicalDevice::EnumerateDeviceExtensionProperties(
    std::vector<VkExtensionProperties>& properties) const {
    uint32_t num;
    VkResult res = dld->vkEnumerateDeviceExtensionProperties(handle, nullptr, &num, nullptr);
    if (res != VK_SUCCESS) return res;

    properties.resize(num);
    return dld->vkEnumerateDeviceExtensionProperties(handle, nullptr, &num, properties.data());
}

VkResult PhysicalDevice::GetSurfaceSupportKHR(uint32_t queue_family_index, VkSurfaceKHR surface,
                                              bool& supported) const {
    VkBool32 vksupported;
    VkResult res = dld->vkGetPhysicalDeviceSurfaceSupportKHR(
        handle, queue_family_index, surface, &vksupported);
    if (res == VK_SUCCESS) supported = vksupported;
    return res;
}

VkResult
PhysicalDevice::GetSurfaceCapabilitiesKHR(VkSurfaceKHR              surface,
                                          VkSurfaceCapabilitiesKHR& capabilities) const noexcept {
    return (dld->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(handle, surface, &capabilities));
}

VkResult PhysicalDevice::GetSurfaceFormatsKHR(VkSurfaceKHR                     surface,
                                              std::vector<VkSurfaceFormatKHR>& formats) const {
    uint32_t num;
    if (auto res = dld->vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface, &num, nullptr);
        res != VK_SUCCESS) {
        return res;
    }
    formats.resize(num);
    return dld->vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface, &num, formats.data());
}

VkResult PhysicalDevice::GetSurfacePresentModesKHR(VkSurfaceKHR                   surface,
                                                   std::vector<VkPresentModeKHR>& modes) const {
    uint32_t num;
    if (auto res = dld->vkGetPhysicalDeviceSurfacePresentModesKHR(handle, surface, &num, nullptr);
        res != VK_SUCCESS) {
        return res;
    }
    modes.resize(num);
    return dld->vkGetPhysicalDeviceSurfacePresentModesKHR(handle, surface, &num, modes.data());
}

VkPhysicalDeviceMemoryProperties2
PhysicalDevice::GetMemoryProperties(void* next_structures) const noexcept {
    VkPhysicalDeviceMemoryProperties2 properties {};
    properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    properties.pNext = next_structures;
    dld->vkGetPhysicalDeviceMemoryProperties2(handle, &properties);
    return properties;
}

std::vector<VkQueueFamilyProperties> PhysicalDevice::GetQueueFamilyProperties() const {
    uint32_t num;
    dld->vkGetPhysicalDeviceQueueFamilyProperties(handle, &num, nullptr);
    std::vector<VkQueueFamilyProperties> properties(num);
    dld->vkGetPhysicalDeviceQueueFamilyProperties(handle, &num, properties.data());
    return properties;
}

VkResult CommandPool::Allocate(std::size_t num_buffers, VkCommandBufferLevel level,
                               CommandBuffers& buffers_) const {
    const VkCommandBufferAllocateInfo ai {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext              = nullptr,
        .commandPool        = handle,
        .level              = level,
        .commandBufferCount = static_cast<uint32_t>(num_buffers),
    };

    std::unique_ptr buffers = std::make_unique<VkCommandBuffer[]>(num_buffers);
    VkResult        res     = dld->vkAllocateCommandBuffers(owner, &ai, buffers.get());
    if (res == VK_SUCCESS)
        buffers_ = CommandBuffers(std::move(buffers), num_buffers, owner, handle, *dld);
    return res;
}

VkResult DeviceMemory::GetMemoryFdKHR(int* fd) const {
    const VkMemoryGetFdInfoKHR get_fd_info {
        .sType      = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR,
        .pNext      = nullptr,
        .memory     = handle,
        .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR,
    };
    return dld->vkGetMemoryFdKHR(owner, &get_fd_info, fd);
}

std::optional<std::vector<VkExtensionProperties>>
EnumerateInstanceExtensionProperties(const InstanceDispatch& dld) {
    uint32_t num;
    if (dld.vkEnumerateInstanceExtensionProperties(nullptr, &num, nullptr) != VK_SUCCESS) {
        return std::nullopt;
    }
    std::vector<VkExtensionProperties> properties(num);
    if (dld.vkEnumerateInstanceExtensionProperties(nullptr, &num, properties.data()) !=
        VK_SUCCESS) {
        return std::nullopt;
    }
    return properties;
}

std::optional<std::vector<VkLayerProperties>>
EnumerateInstanceLayerProperties(const InstanceDispatch& dld) {
    uint32_t num;
    if (dld.vkEnumerateInstanceLayerProperties(&num, nullptr) != VK_SUCCESS) {
        return std::nullopt;
    }
    std::vector<VkLayerProperties> properties(num);
    if (dld.vkEnumerateInstanceLayerProperties(&num, properties.data()) != VK_SUCCESS) {
        return std::nullopt;
    }
    return properties;
}

// clang-format off
const char* ToString(VkResult result) noexcept {
    switch (result) {
    case VkResult::VK_SUCCESS:
        return "VK_SUCCESS";
    case VkResult::VK_NOT_READY:
        return "VK_NOT_READY";
    case VkResult::VK_TIMEOUT:
        return "VK_TIMEOUT";
    case VkResult::VK_EVENT_SET:
        return "VK_EVENT_SET";
    case VkResult::VK_EVENT_RESET:
        return "VK_EVENT_RESET";
    case VkResult::VK_INCOMPLETE:
        return "VK_INCOMPLETE";
    case VkResult::VK_ERROR_OUT_OF_HOST_MEMORY:
        return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VkResult::VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VkResult::VK_ERROR_INITIALIZATION_FAILED:
        return "VK_ERROR_INITIALIZATION_FAILED";
    case VkResult::VK_ERROR_DEVICE_LOST:
        return "VK_ERROR_DEVICE_LOST";
    case VkResult::VK_ERROR_MEMORY_MAP_FAILED:
        return "VK_ERROR_MEMORY_MAP_FAILED";
    case VkResult::VK_ERROR_LAYER_NOT_PRESENT:
        return "VK_ERROR_LAYER_NOT_PRESENT";
    case VkResult::VK_ERROR_EXTENSION_NOT_PRESENT:
        return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VkResult::VK_ERROR_FEATURE_NOT_PRESENT:
        return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VkResult::VK_ERROR_INCOMPATIBLE_DRIVER:
        return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VkResult::VK_ERROR_TOO_MANY_OBJECTS:
        return "VK_ERROR_TOO_MANY_OBJECTS";
    case VkResult::VK_ERROR_FORMAT_NOT_SUPPORTED:
        return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VkResult::VK_ERROR_FRAGMENTED_POOL:
        return "VK_ERROR_FRAGMENTED_POOL";
    case VkResult::VK_ERROR_OUT_OF_POOL_MEMORY:
        return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VkResult::VK_ERROR_INVALID_EXTERNAL_HANDLE:
        return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VkResult::VK_ERROR_SURFACE_LOST_KHR:
        return "VK_ERROR_SURFACE_LOST_KHR";
    case VkResult::VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VkResult::VK_SUBOPTIMAL_KHR:
        return "VK_SUBOPTIMAL_KHR";
    case VkResult::VK_ERROR_OUT_OF_DATE_KHR:
        return "VK_ERROR_OUT_OF_DATE_KHR";
    case VkResult::VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VkResult::VK_ERROR_VALIDATION_FAILED_EXT:
        return "VK_ERROR_VALIDATION_FAILED_EXT";
    case VkResult::VK_ERROR_INVALID_SHADER_NV:
        return "VK_ERROR_INVALID_SHADER_NV";
    case VkResult::VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
        return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
    case VkResult::VK_ERROR_FRAGMENTATION_EXT:
        return "VK_ERROR_FRAGMENTATION_EXT";
    case VkResult::VK_ERROR_NOT_PERMITTED_EXT:
        return "VK_ERROR_NOT_PERMITTED_EXT";
    case VkResult::VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
        return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT";
    case VkResult::VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case VkResult::VK_ERROR_UNKNOWN:
        return "VK_ERROR_UNKNOWN";
    case VkResult::VK_THREAD_IDLE_KHR:
        return "VK_THREAD_IDLE_KHR";
    case VkResult::VK_THREAD_DONE_KHR:
        return "VK_THREAD_DONE_KHR";
    case VkResult::VK_OPERATION_DEFERRED_KHR:
        return "VK_OPERATION_DEFERRED_KHR";
    case VkResult::VK_OPERATION_NOT_DEFERRED_KHR:
        return "VK_OPERATION_NOT_DEFERRED_KHR";
    case VkResult::VK_PIPELINE_COMPILE_REQUIRED_EXT:
        return "VK_PIPELINE_COMPILE_REQUIRED_EXT";
    case VkResult::VK_RESULT_MAX_ENUM:
        return "VK_RESULT_MAX_ENUM";
    }
    return "Unknown";
}

const char* ToString(VkFormat format) noexcept {
    #define X(str) case VkFormat::VK_FORMAT_##str: return "VK_FORMAT_##str";
    switch (format) {
        X(UNDEFINED);
        X(R4G4_UNORM_PACK8);
        X(R4G4B4A4_UNORM_PACK16);
        X(B4G4R4A4_UNORM_PACK16);
        X(R5G6B5_UNORM_PACK16);
        X(B5G6R5_UNORM_PACK16);
        X(R5G5B5A1_UNORM_PACK16);
        X(B5G5R5A1_UNORM_PACK16);
        X(A1R5G5B5_UNORM_PACK16);
        X(R8_UNORM);
        X(R8_SNORM);
        X(R8_USCALED);
        X(R8_SSCALED);
        X(R8_UINT);
        X(R8_SINT);
        X(R8_SRGB);
        X(R8G8_UNORM);
        X(R8G8_SNORM);
        X(R8G8_USCALED);
        X(R8G8_SSCALED);
        X(R8G8_UINT);
        X(R8G8_SINT);
        X(R8G8_SRGB);
        X(R8G8B8_UNORM);
        X(R8G8B8_SNORM);
        X(R8G8B8_USCALED);
        X(R8G8B8_SSCALED);
        X(R8G8B8_UINT);
        X(R8G8B8_SINT);
        X(R8G8B8_SRGB);
        X(B8G8R8_UNORM);
        X(B8G8R8_SNORM);
        X(B8G8R8_USCALED);
        X(B8G8R8_SSCALED);
        X(B8G8R8_UINT);
        X(B8G8R8_SINT);
        X(B8G8R8_SRGB);
        X(R8G8B8A8_UNORM);
        X(R8G8B8A8_SNORM);
        X(R8G8B8A8_USCALED);
        X(R8G8B8A8_SSCALED);
        X(R8G8B8A8_UINT);
        X(R8G8B8A8_SINT);
        X(R8G8B8A8_SRGB);
        X(B8G8R8A8_UNORM);
        X(B8G8R8A8_SNORM);
        X(B8G8R8A8_USCALED);
        X(B8G8R8A8_SSCALED);
        X(B8G8R8A8_UINT);
        X(B8G8R8A8_SINT);
        X(B8G8R8A8_SRGB);
        X(A8B8G8R8_UNORM_PACK32);
        X(A8B8G8R8_SNORM_PACK32);
        X(A8B8G8R8_USCALED_PACK32);
        X(A8B8G8R8_SSCALED_PACK32);
        X(A8B8G8R8_UINT_PACK32);
        X(A8B8G8R8_SINT_PACK32);
        X(A8B8G8R8_SRGB_PACK32);
        X(A2R10G10B10_UNORM_PACK32);
        X(A2R10G10B10_SNORM_PACK32);
        X(A2R10G10B10_USCALED_PACK32);
        X(A2R10G10B10_SSCALED_PACK32);
        X(A2R10G10B10_UINT_PACK32);
        X(A2R10G10B10_SINT_PACK32);
        X(A2B10G10R10_UNORM_PACK32);
        X(A2B10G10R10_SNORM_PACK32);
        X(A2B10G10R10_USCALED_PACK32);
        X(A2B10G10R10_SSCALED_PACK32);
        X(A2B10G10R10_UINT_PACK32);
        X(A2B10G10R10_SINT_PACK32);
        X(R16_UNORM);
        X(R16_SNORM);
        X(R16_USCALED);
        X(R16_SSCALED);
        X(R16_UINT);
        X(R16_SINT);
        X(R16_SFLOAT);
        X(R16G16_UNORM);
        X(R16G16_SNORM);
        X(R16G16_USCALED);
        X(R16G16_SSCALED);
        X(R16G16_UINT);
        X(R16G16_SINT);
        X(R16G16_SFLOAT);
        X(R16G16B16_UNORM);
        X(R16G16B16_SNORM);
        X(R16G16B16_USCALED);
        X(R16G16B16_SSCALED);
        X(R16G16B16_UINT);
        X(R16G16B16_SINT);
        X(R16G16B16_SFLOAT);
        X(R16G16B16A16_UNORM);
        X(R16G16B16A16_SNORM);
        X(R16G16B16A16_USCALED);
        X(R16G16B16A16_SSCALED);
        X(R16G16B16A16_UINT);
        X(R16G16B16A16_SINT);
        X(R16G16B16A16_SFLOAT);
        X(R32_UINT);
        X(R32_SINT);
        X(R32_SFLOAT);
        X(R32G32_UINT);
        X(R32G32_SINT);
        X(R32G32_SFLOAT);
        X(R32G32B32_UINT);
        X(R32G32B32_SINT);
        X(R32G32B32_SFLOAT);
        X(R32G32B32A32_UINT);
        X(R32G32B32A32_SINT);
        X(R32G32B32A32_SFLOAT);
        X(R64_UINT);
        X(R64_SINT);
        X(R64_SFLOAT);
        X(R64G64_UINT);
        X(R64G64_SINT);
        X(R64G64_SFLOAT);
        X(R64G64B64_UINT);
        X(R64G64B64_SINT);
        X(R64G64B64_SFLOAT);
        X(R64G64B64A64_UINT);
        X(R64G64B64A64_SINT);
        X(R64G64B64A64_SFLOAT);
        X(B10G11R11_UFLOAT_PACK32);
        X(E5B9G9R9_UFLOAT_PACK32);
        X(D16_UNORM);
        X(X8_D24_UNORM_PACK32);
        X(D32_SFLOAT);
        X(S8_UINT);
        X(D16_UNORM_S8_UINT);
        X(D24_UNORM_S8_UINT);
        X(D32_SFLOAT_S8_UINT);
        X(BC1_RGB_UNORM_BLOCK);
        X(BC1_RGB_SRGB_BLOCK);
        X(BC1_RGBA_UNORM_BLOCK);
        X(BC1_RGBA_SRGB_BLOCK);
        X(BC2_UNORM_BLOCK);
        X(BC2_SRGB_BLOCK);
        X(BC3_UNORM_BLOCK);
        X(BC3_SRGB_BLOCK);
        X(BC4_UNORM_BLOCK);
        X(BC4_SNORM_BLOCK);
        X(BC5_UNORM_BLOCK);
        X(BC5_SNORM_BLOCK);
        X(BC6H_UFLOAT_BLOCK);
        X(BC6H_SFLOAT_BLOCK);
        X(BC7_UNORM_BLOCK);
        X(BC7_SRGB_BLOCK);
        X(ETC2_R8G8B8_UNORM_BLOCK);
        X(ETC2_R8G8B8_SRGB_BLOCK);
        X(ETC2_R8G8B8A1_UNORM_BLOCK);
        X(ETC2_R8G8B8A1_SRGB_BLOCK);
        X(ETC2_R8G8B8A8_UNORM_BLOCK);
        X(ETC2_R8G8B8A8_SRGB_BLOCK);
        X(EAC_R11_UNORM_BLOCK);
        X(EAC_R11_SNORM_BLOCK);
        X(EAC_R11G11_UNORM_BLOCK);
        X(EAC_R11G11_SNORM_BLOCK);
        X(ASTC_4x4_UNORM_BLOCK);
        X(ASTC_4x4_SRGB_BLOCK);
        X(ASTC_5x4_UNORM_BLOCK);
        X(ASTC_5x4_SRGB_BLOCK);
        X(ASTC_5x5_UNORM_BLOCK);
        X(ASTC_5x5_SRGB_BLOCK);
        X(ASTC_6x5_UNORM_BLOCK);
        X(ASTC_6x5_SRGB_BLOCK);
        X(ASTC_6x6_UNORM_BLOCK);
        X(ASTC_6x6_SRGB_BLOCK);
        X(ASTC_8x5_UNORM_BLOCK);
        X(ASTC_8x5_SRGB_BLOCK);
        X(ASTC_8x6_UNORM_BLOCK);
        X(ASTC_8x6_SRGB_BLOCK);
        X(ASTC_8x8_UNORM_BLOCK);
        X(ASTC_8x8_SRGB_BLOCK);
        X(ASTC_10x5_UNORM_BLOCK);
        X(ASTC_10x5_SRGB_BLOCK);
        X(ASTC_10x6_UNORM_BLOCK);
        X(ASTC_10x6_SRGB_BLOCK);
        X(ASTC_10x8_UNORM_BLOCK);
        X(ASTC_10x8_SRGB_BLOCK);
        X(ASTC_10x10_UNORM_BLOCK);
        X(ASTC_10x10_SRGB_BLOCK);
        X(ASTC_12x10_UNORM_BLOCK);
        X(ASTC_12x10_SRGB_BLOCK);
        X(ASTC_12x12_UNORM_BLOCK);
        X(ASTC_12x12_SRGB_BLOCK);
        X(G8B8G8R8_422_UNORM);
        X(B8G8R8G8_422_UNORM);
        X(G8_B8_R8_3PLANE_420_UNORM);
        X(G8_B8R8_2PLANE_420_UNORM);
        X(G8_B8_R8_3PLANE_422_UNORM);
        X(G8_B8R8_2PLANE_422_UNORM);
        X(G8_B8_R8_3PLANE_444_UNORM);
        X(R10X6_UNORM_PACK16);
        X(R10X6G10X6_UNORM_2PACK16);
        X(R10X6G10X6B10X6A10X6_UNORM_4PACK16);
        X(G10X6B10X6G10X6R10X6_422_UNORM_4PACK16);
        X(B10X6G10X6R10X6G10X6_422_UNORM_4PACK16);
        X(G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16);
        X(G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16);
        X(G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16);
        X(G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16);
        X(G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16);
        X(R12X4_UNORM_PACK16);
        X(R12X4G12X4_UNORM_2PACK16);
        X(R12X4G12X4B12X4A12X4_UNORM_4PACK16);
        X(G12X4B12X4G12X4R12X4_422_UNORM_4PACK16);
        X(B12X4G12X4R12X4G12X4_422_UNORM_4PACK16);
        X(G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16);
        X(G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16);
        X(G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16);
        X(G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16);
        X(G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16);
        X(G16B16G16R16_422_UNORM);
        X(B16G16R16G16_422_UNORM);
        X(G16_B16_R16_3PLANE_420_UNORM);
        X(G16_B16R16_2PLANE_420_UNORM);
        X(G16_B16_R16_3PLANE_422_UNORM);
        X(G16_B16R16_2PLANE_422_UNORM);
        X(G16_B16_R16_3PLANE_444_UNORM);
        X(G8_B8R8_2PLANE_444_UNORM);
        X(G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16);
        X(G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16);
        X(G16_B16R16_2PLANE_444_UNORM);
        X(A4R4G4B4_UNORM_PACK16);
        X(A4B4G4R4_UNORM_PACK16);
        X(ASTC_4x4_SFLOAT_BLOCK);
        X(ASTC_5x4_SFLOAT_BLOCK);
        X(ASTC_5x5_SFLOAT_BLOCK);
        X(ASTC_6x5_SFLOAT_BLOCK);
        X(ASTC_6x6_SFLOAT_BLOCK);
        X(ASTC_8x5_SFLOAT_BLOCK);
        X(ASTC_8x6_SFLOAT_BLOCK);
        X(ASTC_8x8_SFLOAT_BLOCK);
        X(ASTC_10x5_SFLOAT_BLOCK);
        X(ASTC_10x6_SFLOAT_BLOCK);
        X(ASTC_10x8_SFLOAT_BLOCK);
        X(ASTC_10x10_SFLOAT_BLOCK);
        X(ASTC_12x10_SFLOAT_BLOCK);
        X(ASTC_12x12_SFLOAT_BLOCK);
        X(PVRTC1_2BPP_UNORM_BLOCK_IMG);
        X(PVRTC1_4BPP_UNORM_BLOCK_IMG);
        X(PVRTC2_2BPP_UNORM_BLOCK_IMG);
        X(PVRTC2_4BPP_UNORM_BLOCK_IMG);
        X(PVRTC1_2BPP_SRGB_BLOCK_IMG);
        X(PVRTC1_4BPP_SRGB_BLOCK_IMG);
        X(PVRTC2_2BPP_SRGB_BLOCK_IMG);
        X(PVRTC2_4BPP_SRGB_BLOCK_IMG);
        X(MAX_ENUM);
    }
    #undef X
    return "VK_FORMAT_UNKNOWN";
}
const char* ToString(VkColorSpaceKHR o) noexcept {
    #define X(str) case VkColorSpaceKHR::VK_##str: return "VK_##str";
    switch (o) {
        X(COLOR_SPACE_SRGB_NONLINEAR_KHR);
        X(COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT);
        X(COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT);
        X(COLOR_SPACE_DISPLAY_P3_LINEAR_EXT);
        X(COLOR_SPACE_DCI_P3_NONLINEAR_EXT);
        X(COLOR_SPACE_BT709_LINEAR_EXT);
        X(COLOR_SPACE_BT709_NONLINEAR_EXT);
        X(COLOR_SPACE_BT2020_LINEAR_EXT);
        X(COLOR_SPACE_HDR10_ST2084_EXT);
        X(COLOR_SPACE_DOLBYVISION_EXT);
        X(COLOR_SPACE_HDR10_HLG_EXT);
        X(COLOR_SPACE_ADOBERGB_LINEAR_EXT);
        X(COLOR_SPACE_ADOBERGB_NONLINEAR_EXT);
        X(COLOR_SPACE_PASS_THROUGH_EXT);
        X(COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT);
        X(COLOR_SPACE_DISPLAY_NATIVE_AMD);
        X(COLOR_SPACE_MAX_ENUM_KHR);
    }
    #undef X
    return "VK_FORMAT_UNKNOWN";
}
// cX(lang-format on

} //X( namespace vvk
