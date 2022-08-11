#pragma once

#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>
#include <cassert>

// vulkan loader dynamically
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include "handle.hpp"
#include "span.hpp"
#include "Core/NoCopyMove.hpp"
#include "Utils/DynamicLibrary.hpp"
#include "Utils/Logging.h"

namespace vvk
{
const char* ToString(VkResult result) noexcept;

const char* ToString(VkFormat format) noexcept;

const char* ToString(VkColorSpaceKHR color) noexcept;

} // namespace vvk

#define VVK_CHECK(f)         VVK_CHECK_ACT(, f)
#define VVK_CHECK_BOOL_RE(f) VVK_CHECK_ACT(return false, f)
#define VVK_CHECK_VOID_RE(f) VVK_CHECK_ACT(return, f)
#define VVK_CHECK_RE(f)      VVK_CHECK_ACT(return _res, f)
#define VVK_CHECK_ACT(act, f)                                     \
    {                                                             \
        VkResult _res = (f);                                      \
        if (_res != VK_SUCCESS) {                                 \
            LOG_ERROR("VkResult is \"%s\"", vvk::ToString(_res)); \
            assert(_res == VK_SUCCESS);                           \
            { act; };                                             \
        }                                                         \
    }

namespace vvk
{
struct InstanceDispatch {
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr {};

    // global commands
    PFN_vkCreateInstance                       vkCreateInstance {};
    PFN_vkDestroyInstance                      vkDestroyInstance {};
    PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties {};
    PFN_vkEnumerateInstanceLayerProperties     vkEnumerateInstanceLayerProperties {};

    // instance local
    PFN_vkCreateDebugUtilsMessengerEXT            vkCreateDebugUtilsMessengerEXT {};
    PFN_vkCreateDevice                            vkCreateDevice {};
    PFN_vkDestroyDebugUtilsMessengerEXT           vkDestroyDebugUtilsMessengerEXT {};
    PFN_vkDestroyDevice                           vkDestroyDevice {};
    PFN_vkDestroySurfaceKHR                       vkDestroySurfaceKHR {};
    PFN_vkEnumerateDeviceExtensionProperties      vkEnumerateDeviceExtensionProperties {};
    PFN_vkEnumeratePhysicalDevices                vkEnumeratePhysicalDevices {};
    PFN_vkGetDeviceProcAddr                       vkGetDeviceProcAddr {};
    PFN_vkGetPhysicalDeviceFeatures2KHR           vkGetPhysicalDeviceFeatures2KHR {};
    PFN_vkGetPhysicalDeviceFormatProperties       vkGetPhysicalDeviceFormatProperties {};
    PFN_vkGetPhysicalDeviceMemoryProperties       vkGetPhysicalDeviceMemoryProperties {};
    PFN_vkGetPhysicalDeviceMemoryProperties2      vkGetPhysicalDeviceMemoryProperties2 {};
    PFN_vkGetPhysicalDeviceProperties             vkGetPhysicalDeviceProperties {};
    PFN_vkGetPhysicalDeviceProperties2KHR         vkGetPhysicalDeviceProperties2KHR {};
    PFN_vkGetPhysicalDeviceQueueFamilyProperties  vkGetPhysicalDeviceQueueFamilyProperties {};
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR {};
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR      vkGetPhysicalDeviceSurfaceFormatsKHR {};
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR {};
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR      vkGetPhysicalDeviceSurfaceSupportKHR {};
    PFN_vkGetSwapchainImagesKHR                   vkGetSwapchainImagesKHR {};
    PFN_vkQueuePresentKHR                         vkQueuePresentKHR {};
};

struct DeviceDispatch : InstanceDispatch {
    PFN_vkAcquireNextImageKHR                 vkAcquireNextImageKHR {};
    PFN_vkAllocateCommandBuffers              vkAllocateCommandBuffers {};
    PFN_vkAllocateDescriptorSets              vkAllocateDescriptorSets {};
    PFN_vkAllocateMemory                      vkAllocateMemory {};
    PFN_vkBeginCommandBuffer                  vkBeginCommandBuffer {};
    PFN_vkBindBufferMemory                    vkBindBufferMemory {};
    PFN_vkBindImageMemory                     vkBindImageMemory {};
    PFN_vkCmdBeginDebugUtilsLabelEXT          vkCmdBeginDebugUtilsLabelEXT {};
    PFN_vkCmdBeginQuery                       vkCmdBeginQuery {};
    PFN_vkCmdBeginRenderPass                  vkCmdBeginRenderPass {};
    PFN_vkCmdBeginTransformFeedbackEXT        vkCmdBeginTransformFeedbackEXT {};
    PFN_vkCmdBindDescriptorSets               vkCmdBindDescriptorSets {};
    PFN_vkCmdBindIndexBuffer                  vkCmdBindIndexBuffer {};
    PFN_vkCmdBindPipeline                     vkCmdBindPipeline {};
    PFN_vkCmdBindTransformFeedbackBuffersEXT  vkCmdBindTransformFeedbackBuffersEXT {};
    PFN_vkCmdBindVertexBuffers                vkCmdBindVertexBuffers {};
    PFN_vkCmdBindVertexBuffers2EXT            vkCmdBindVertexBuffers2EXT {};
    PFN_vkCmdBlitImage                        vkCmdBlitImage {};
    PFN_vkCmdClearColorImage                  vkCmdClearColorImage {};
    PFN_vkCmdClearAttachments                 vkCmdClearAttachments {};
    PFN_vkCmdCopyBuffer                       vkCmdCopyBuffer {};
    PFN_vkCmdCopyBufferToImage                vkCmdCopyBufferToImage {};
    PFN_vkCmdCopyImage                        vkCmdCopyImage {};
    PFN_vkCmdCopyImageToBuffer                vkCmdCopyImageToBuffer {};
    PFN_vkCmdDispatch                         vkCmdDispatch {};
    PFN_vkCmdDraw                             vkCmdDraw {};
    PFN_vkCmdDrawIndexed                      vkCmdDrawIndexed {};
    PFN_vkCmdEndDebugUtilsLabelEXT            vkCmdEndDebugUtilsLabelEXT {};
    PFN_vkCmdEndQuery                         vkCmdEndQuery {};
    PFN_vkCmdEndRenderPass                    vkCmdEndRenderPass {};
    PFN_vkCmdEndTransformFeedbackEXT          vkCmdEndTransformFeedbackEXT {};
    PFN_vkCmdFillBuffer                       vkCmdFillBuffer {};
    PFN_vkCmdPipelineBarrier                  vkCmdPipelineBarrier {};
    PFN_vkCmdPushConstants                    vkCmdPushConstants {};
    PFN_vkCmdPushDescriptorSetKHR             vkCmdPushDescriptorSetKHR {};
    PFN_vkCmdPushDescriptorSetWithTemplateKHR vkCmdPushDescriptorSetWithTemplateKHR {};
    PFN_vkCmdResolveImage                     vkCmdResolveImage {};
    PFN_vkCmdSetBlendConstants                vkCmdSetBlendConstants {};
    PFN_vkCmdSetCullModeEXT                   vkCmdSetCullModeEXT {};
    PFN_vkCmdSetDepthBias                     vkCmdSetDepthBias {};
    PFN_vkCmdSetDepthBounds                   vkCmdSetDepthBounds {};
    PFN_vkCmdSetDepthBoundsTestEnableEXT      vkCmdSetDepthBoundsTestEnableEXT {};
    PFN_vkCmdSetDepthCompareOpEXT             vkCmdSetDepthCompareOpEXT {};
    PFN_vkCmdSetDepthTestEnableEXT            vkCmdSetDepthTestEnableEXT {};
    PFN_vkCmdSetDepthWriteEnableEXT           vkCmdSetDepthWriteEnableEXT {};
    PFN_vkCmdSetEvent                         vkCmdSetEvent {};
    PFN_vkCmdSetFrontFaceEXT                  vkCmdSetFrontFaceEXT {};
    PFN_vkCmdSetLineWidth                     vkCmdSetLineWidth {};
    PFN_vkCmdSetPrimitiveTopologyEXT          vkCmdSetPrimitiveTopologyEXT {};
    PFN_vkCmdSetScissor                       vkCmdSetScissor {};
    PFN_vkCmdSetStencilCompareMask            vkCmdSetStencilCompareMask {};
    PFN_vkCmdSetStencilOpEXT                  vkCmdSetStencilOpEXT {};
    PFN_vkCmdSetStencilReference              vkCmdSetStencilReference {};
    PFN_vkCmdSetStencilTestEnableEXT          vkCmdSetStencilTestEnableEXT {};
    PFN_vkCmdSetStencilWriteMask              vkCmdSetStencilWriteMask {};
    PFN_vkCmdSetVertexInputEXT                vkCmdSetVertexInputEXT {};
    PFN_vkCmdSetViewport                      vkCmdSetViewport {};
    PFN_vkCmdWaitEvents                       vkCmdWaitEvents {};
    PFN_vkCreateBuffer                        vkCreateBuffer {};
    PFN_vkCreateBufferView                    vkCreateBufferView {};
    PFN_vkCreateCommandPool                   vkCreateCommandPool {};
    PFN_vkCreateComputePipelines              vkCreateComputePipelines {};
    PFN_vkCreateDescriptorPool                vkCreateDescriptorPool {};
    PFN_vkCreateDescriptorSetLayout           vkCreateDescriptorSetLayout {};
    PFN_vkCreateDescriptorUpdateTemplateKHR   vkCreateDescriptorUpdateTemplateKHR {};
    PFN_vkCreateEvent                         vkCreateEvent {};
    PFN_vkCreateFence                         vkCreateFence {};
    PFN_vkCreateFramebuffer                   vkCreateFramebuffer {};
    PFN_vkCreateGraphicsPipelines             vkCreateGraphicsPipelines {};
    PFN_vkCreateImage                         vkCreateImage {};
    PFN_vkCreateImageView                     vkCreateImageView {};
    PFN_vkCreatePipelineLayout                vkCreatePipelineLayout {};
    PFN_vkCreateQueryPool                     vkCreateQueryPool {};
    PFN_vkCreateRenderPass                    vkCreateRenderPass {};
    PFN_vkCreateSampler                       vkCreateSampler {};
    PFN_vkCreateSemaphore                     vkCreateSemaphore {};
    PFN_vkCreateShaderModule                  vkCreateShaderModule {};
    PFN_vkCreateSwapchainKHR                  vkCreateSwapchainKHR {};
    PFN_vkDestroyBuffer                       vkDestroyBuffer {};
    PFN_vkDestroyBufferView                   vkDestroyBufferView {};
    PFN_vkDestroyCommandPool                  vkDestroyCommandPool {};
    PFN_vkDestroyDescriptorPool               vkDestroyDescriptorPool {};
    PFN_vkDestroyDescriptorSetLayout          vkDestroyDescriptorSetLayout {};
    PFN_vkDestroyDescriptorUpdateTemplateKHR  vkDestroyDescriptorUpdateTemplateKHR {};
    PFN_vkDestroyEvent                        vkDestroyEvent {};
    PFN_vkDestroyFence                        vkDestroyFence {};
    PFN_vkDestroyFramebuffer                  vkDestroyFramebuffer {};
    PFN_vkDestroyImage                        vkDestroyImage {};
    PFN_vkDestroyImageView                    vkDestroyImageView {};
    PFN_vkDestroyPipeline                     vkDestroyPipeline {};
    PFN_vkDestroyPipelineLayout               vkDestroyPipelineLayout {};
    PFN_vkDestroyQueryPool                    vkDestroyQueryPool {};
    PFN_vkDestroyRenderPass                   vkDestroyRenderPass {};
    PFN_vkDestroySampler                      vkDestroySampler {};
    PFN_vkDestroySemaphore                    vkDestroySemaphore {};
    PFN_vkDestroyShaderModule                 vkDestroyShaderModule {};
    PFN_vkDestroySwapchainKHR                 vkDestroySwapchainKHR {};
    PFN_vkDeviceWaitIdle                      vkDeviceWaitIdle {};
    PFN_vkEndCommandBuffer                    vkEndCommandBuffer {};
    PFN_vkFreeCommandBuffers                  vkFreeCommandBuffers {};
    PFN_vkFreeDescriptorSets                  vkFreeDescriptorSets {};
    PFN_vkFreeMemory                          vkFreeMemory {};
    PFN_vkGetBufferMemoryRequirements2        vkGetBufferMemoryRequirements2 {};
    PFN_vkGetDeviceQueue                      vkGetDeviceQueue {};
    PFN_vkGetEventStatus                      vkGetEventStatus {};
    PFN_vkGetFenceStatus                      vkGetFenceStatus {};
    PFN_vkGetImageMemoryRequirements          vkGetImageMemoryRequirements {};
    PFN_vkGetMemoryFdKHR                      vkGetMemoryFdKHR {};
    PFN_vkGetPipelineExecutablePropertiesKHR  vkGetPipelineExecutablePropertiesKHR {};
    PFN_vkGetPipelineExecutableStatisticsKHR  vkGetPipelineExecutableStatisticsKHR {};
    PFN_vkGetQueryPoolResults                 vkGetQueryPoolResults {};
    PFN_vkGetSemaphoreCounterValueKHR         vkGetSemaphoreCounterValueKHR {};
    PFN_vkMapMemory                           vkMapMemory {};
    PFN_vkQueueSubmit                         vkQueueSubmit {};
    PFN_vkResetFences                         vkResetFences {};
    PFN_vkResetQueryPoolEXT                   vkResetQueryPoolEXT {};
    PFN_vkSetDebugUtilsObjectNameEXT          vkSetDebugUtilsObjectNameEXT {};
    PFN_vkSetDebugUtilsObjectTagEXT           vkSetDebugUtilsObjectTagEXT {};
    PFN_vkUnmapMemory                         vkUnmapMemory {};
    PFN_vkUpdateDescriptorSetWithTemplateKHR  vkUpdateDescriptorSetWithTemplateKHR {};
    PFN_vkUpdateDescriptorSets                vkUpdateDescriptorSets {};
    PFN_vkWaitForFences                       vkWaitForFences {};
    PFN_vkWaitSemaphoresKHR                   vkWaitSemaphoresKHR {};
};

template<typename THandle, typename Type = typename THandle::handle_type>
std::vector<Type> ToVector(std::span<THandle> handles) {
    std::vector<Type> res(handles.size());
    std::transform(handles.begin(), handles.end(), res.begin(), [](const auto& h) {
        return *h;
    });
    return res;
}

/// Array of a pool allocation.
/// Analogue to std::vector
template<typename AllocationType, typename PoolType>
class PoolAllocations {
public:
    /// Construct an empty allocation.
    PoolAllocations() = default;

    /// Construct an allocation. Errors are reported through IsOutOfPoolMemory().
    explicit PoolAllocations(std::unique_ptr<AllocationType[]> allocations_, std::size_t num_,
                             VkDevice device_, PoolType pool_, const DeviceDispatch& dld_) noexcept
        : allocations { std::move(allocations_) },
          num { num_ },
          device { device_ },
          pool { pool_ },
          dld { &dld_ } {}

    /// Copying Vulkan allocations is not supported and will never be.
    PoolAllocations(const PoolAllocations&)            = delete;
    PoolAllocations& operator=(const PoolAllocations&) = delete;

    /// Construct an allocation transfering ownership from another allocation.
    PoolAllocations(PoolAllocations&& rhs) noexcept
        : allocations { std::move(rhs.allocations) },
          num { rhs.num },
          device { rhs.device },
          pool { rhs.pool },
          dld { rhs.dld } {}

    /// Assign an allocation transfering ownership from another allocation.
    /// Releases any previously held allocation.
    PoolAllocations& operator=(PoolAllocations&& rhs) noexcept {
        Release();
        allocations = std::move(rhs.allocations);
        num         = rhs.num;
        device      = rhs.device;
        pool        = rhs.pool;
        dld         = rhs.dld;
        return *this;
    }

    /// Destroys any held allocation.
    ~PoolAllocations() { Release(); }

    /// Returns the number of allocations.
    std::size_t size() const noexcept { return num; }

    /// Returns a pointer to the array of allocations.
    AllocationType const* data() const noexcept { return allocations.get(); }

    /// Returns the allocation in the specified index.
    /// @pre index < size()
    AllocationType operator[](std::size_t index) const noexcept { return allocations[index]; }

    /// True when a pool fails to construct.
    bool IsOutOfPoolMemory() const noexcept { return ! device; }

private:
    /// Destroys the held allocations if they exist.
    void Release() noexcept {
        if (! allocations) {
            return;
        }
        const Span<AllocationType> span(allocations.get(), num);
        VVK_CHECK(Free(device, pool, span, *dld));
    }

    std::unique_ptr<AllocationType[]> allocations;
    std::size_t                       num    = 0;
    VkDevice                          device = nullptr;
    PoolType                          pool   = nullptr;
    const DeviceDispatch*             dld    = nullptr;
};

VkResult LoadLibrary(utils::DynamicLibrary&, vvk::InstanceDispatch&);

/// reauire vkGetInstanceProcAddr valid
bool Load(InstanceDispatch&) noexcept;

bool Load(VkInstance, InstanceDispatch&) noexcept;
bool Load(VkDevice, InstanceDispatch&) noexcept;

void Destroy(VkInstance, const InstanceDispatch&) noexcept;
void Destroy(VkDevice, const InstanceDispatch&) noexcept;
void Destroy(VkInstance, VkDebugUtilsMessengerEXT, const InstanceDispatch&) noexcept;
void Destroy(VkInstance, VkSurfaceKHR, const InstanceDispatch&) noexcept;
void Destroy(VkDevice, VkCommandPool, const DeviceDispatch&) noexcept;
void Destroy(VkDevice, VkPipeline, const DeviceDispatch&) noexcept;
void Destroy(VkDevice, VkPipelineLayout, const DeviceDispatch&) noexcept;
void Destroy(VkDevice, VkRenderPass, const DeviceDispatch&) noexcept;
void Destroy(VkDevice, VkDescriptorSetLayout, const DeviceDispatch&) noexcept;
void Destroy(VkDevice, VkImage, const DeviceDispatch&) noexcept;
void Destroy(VkDevice, VkImageView, const DeviceDispatch&) noexcept;
void Destroy(VkDevice, VkDeviceMemory, const DeviceDispatch&) noexcept;
void Destroy(VkDevice, VkShaderModule, const DeviceDispatch&) noexcept;
void Destroy(VkDevice, VkSwapchainKHR, const DeviceDispatch&) noexcept;
void Destroy(VkDevice, VkSampler, const DeviceDispatch&) noexcept;
void Destroy(VkDevice, VkSemaphore, const DeviceDispatch&) noexcept;
void Destroy(VkDevice, VkFence, const DeviceDispatch&) noexcept;
void Destroy(VkDevice, VkFramebuffer, const DeviceDispatch&) noexcept;

VkResult Free(VkDevice, VkCommandPool, Span<VkCommandBuffer>, const DeviceDispatch&) noexcept;

using DebugUtilsMessenger = Handle<VkDebugUtilsMessengerEXT, VkInstance, InstanceDispatch>;
using DescriptorSetLayout = Handle<VkDescriptorSetLayout, VkDevice, DeviceDispatch>;
using SurfaceKHR          = Handle<VkSurfaceKHR, VkInstance, InstanceDispatch>;
using Pipeline            = Handle<VkPipeline, VkDevice, DeviceDispatch>;
using PipelineLayout      = Handle<VkPipelineLayout, VkDevice, DeviceDispatch>;
using RenderPass          = Handle<VkRenderPass, VkDevice, DeviceDispatch>;
using Sampler             = Handle<VkSampler, VkDevice, DeviceDispatch>;

using DescriptorSets = PoolAllocations<VkDescriptorSet, VkDescriptorPool>;
using CommandBuffers = PoolAllocations<VkCommandBuffer, VkCommandPool>;

class PhysicalDevice;

class Instance : public Handle<VkInstance, NoOwner, InstanceDispatch> {
    using Handle<VkInstance, NoOwner, InstanceDispatch>::Handle;

public:
    static VkResult Create(Instance&, const VkApplicationInfo&, Span<const char*> layers,
                           Span<const char*> extensions, InstanceDispatch&) noexcept;

    std::vector<PhysicalDevice> EnumeratePhysicalDevices() const noexcept;

    DebugUtilsMessenger
    CreateDebugUtilsMessenger(const VkDebugUtilsMessengerCreateInfoEXT&) const noexcept;

    const InstanceDispatch& Dispatch() const noexcept { return *dld; }
};

class Buffer : public Handle<VkBuffer, VkDevice, DeviceDispatch> {
    using Handle<VkBuffer, VkDevice, DeviceDispatch>::Handle;

public:
    VkResult BindMemory(VkDeviceMemory memory, VkDeviceSize offset) const noexcept;
};

class Image : public Handle<VkImage, VkDevice, DeviceDispatch> {
    using Handle<VkImage, VkDevice, DeviceDispatch>::Handle;

public:
    VkResult BindMemory(VkDeviceMemory memory, VkDeviceSize offset) const noexcept;
};

class ImageView : public Handle<VkImageView, VkDevice, DeviceDispatch> {
    using Handle<VkImageView, VkDevice, DeviceDispatch>::Handle;
};

class Queue : public Handle<VkQueue, NoOwnerLife, DeviceDispatch> {
    using Handle<VkQueue, NoOwnerLife, DeviceDispatch>::Handle;

public:
    VkResult Submit(Span<VkSubmitInfo> submit_infos,
                    VkFence            fence = VK_NULL_HANDLE) const noexcept {
        return dld->vkQueueSubmit(
            handle, (uint32_t)submit_infos.size(), submit_infos.data(), fence);
    }

    VkResult Present(const VkPresentInfoKHR& present_info) const noexcept {
        return dld->vkQueuePresentKHR(handle, &present_info);
    }
};

class SwapchainKHR : public Handle<VkSwapchainKHR, VkDevice, DeviceDispatch> {
    using Handle<VkSwapchainKHR, VkDevice, DeviceDispatch>::Handle;

public:
    VkResult GetImages(std::vector<VkImage>&) const;
};

class PhysicalDevice : public Handle<VkPhysicalDevice, NoOwnerLife, InstanceDispatch> {
    using Handle<VkPhysicalDevice, NoOwnerLife, InstanceDispatch>::Handle;

public:
    VkPhysicalDeviceProperties GetProperties() const noexcept;

    void GetProperties2KHR(VkPhysicalDeviceProperties2KHR&) const noexcept;

    VkPhysicalDeviceFeatures GetFeatures() const noexcept;

    void GetFeatures2KHR(VkPhysicalDeviceFeatures2KHR&) const noexcept;

    VkFormatProperties GetFormatProperties(VkFormat) const noexcept;

    VkResult EnumerateDeviceExtensionProperties(std::vector<VkExtensionProperties>&) const;

    std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties() const;

    VkResult GetSurfaceSupportKHR(uint32_t queue_family_index, VkSurfaceKHR, bool&) const;

    VkResult GetSurfaceCapabilitiesKHR(VkSurfaceKHR, VkSurfaceCapabilitiesKHR&) const noexcept;

    VkResult GetSurfaceFormatsKHR(VkSurfaceKHR surface, std::vector<VkSurfaceFormatKHR>&) const;

    VkResult GetSurfacePresentModesKHR(VkSurfaceKHR surface, std::vector<VkPresentModeKHR>&) const;

    VkPhysicalDeviceMemoryProperties2
    GetMemoryProperties(void* next_structures = nullptr) const noexcept;
};

class CommandPool : public Handle<VkCommandPool, VkDevice, DeviceDispatch> {
    using Handle<VkCommandPool, VkDevice, DeviceDispatch>::Handle;

public:
    VkResult Allocate(std::size_t num_buffers, VkCommandBufferLevel level, CommandBuffers&) const;
};

class DeviceMemory : public Handle<VkDeviceMemory, VkDevice, DeviceDispatch> {
    using Handle<VkDeviceMemory, VkDevice, DeviceDispatch>::Handle;

public:
    VkResult GetMemoryFdKHR(int*) const;

    VkResult Map(VkDeviceSize offset, VkDeviceSize size, uint8_t** data) const {
        return (dld->vkMapMemory(owner, handle, offset, size, 0, (void**)data));
    }

    void Unmap() const noexcept { dld->vkUnmapMemory(owner, handle); }
};

class Framebuffer : public Handle<VkFramebuffer, VkDevice, DeviceDispatch> {
    using Handle<VkFramebuffer, VkDevice, DeviceDispatch>::Handle;

public:
};

class ShaderModule : public Handle<VkShaderModule, VkDevice, DeviceDispatch> {
    using Handle<VkShaderModule, VkDevice, DeviceDispatch>::Handle;

public:
};

class Fence : public Handle<VkFence, VkDevice, DeviceDispatch> {
    using Handle<VkFence, VkDevice, DeviceDispatch>::Handle;

public:
    VkResult Wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) const noexcept {
        return dld->vkWaitForFences(owner, 1, &handle, true, timeout);
    }

    VkResult GetStatus() const noexcept { return dld->vkGetFenceStatus(owner, handle); }

    VkResult Reset() const { return dld->vkResetFences(owner, 1, &handle); }
};

class Semaphore : public Handle<VkSemaphore, VkDevice, DeviceDispatch> {
    using Handle<VkSemaphore, VkDevice, DeviceDispatch>::Handle;

public:
    VkResult GetCounter(uint64_t* value) const {
        return dld->vkGetSemaphoreCounterValueKHR(owner, handle, value);
    }

    // VK_TIMEOUT
    VkResult Wait(uint64_t value, uint64_t timeout = std::numeric_limits<uint64_t>::max()) const {
        const VkSemaphoreWaitInfoKHR wait_info {
            .sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO_KHR,
            .pNext          = nullptr,
            .flags          = 0,
            .semaphoreCount = 1,
            .pSemaphores    = &handle,
            .pValues        = &value,
        };
        return dld->vkWaitSemaphoresKHR(owner, &wait_info, timeout);
    }
};

class Device : public Handle<VkDevice, NoOwner, DeviceDispatch> {
    using Handle<VkDevice, NoOwner, DeviceDispatch>::Handle;

public:
    static VkResult Create(Device&, VkPhysicalDevice physical_device,
                           Span<const VkDeviceQueueCreateInfo> queues_ci,
                           Span<const char*> enabled_extensions, const void* next,
                           DeviceDispatch& dispatch);

    Queue GetQueue(uint32_t family_index) const noexcept;

    VkMemoryRequirements GetImageMemoryRequirements(VkImage image) const noexcept;

    VkResult AllocateMemory(const VkMemoryAllocateInfo& ai, DeviceMemory&) const noexcept;

    VkResult CreateCommandPool(const VkCommandPoolCreateInfo& ci, CommandPool&) const;
    VkResult CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& ci,
                                       DescriptorSetLayout&) const noexcept;
    VkResult CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& ci,
                                    Pipeline&) const noexcept;

    VkResult CreateRenderPass(const VkRenderPassCreateInfo& ci, RenderPass&) const noexcept;

    VkResult CreatePipelineLayout(const VkPipelineLayoutCreateInfo& ci,
                                  PipelineLayout&) const noexcept;

    VkResult CreateSwapchainKHR(const VkSwapchainCreateInfoKHR& ci, SwapchainKHR&) const noexcept;

    VkResult CreateShaderModule(const VkShaderModuleCreateInfo& ci, ShaderModule&) const noexcept;

    VkResult CreateSemaphore(const VkSemaphoreCreateInfo& ci, Semaphore&) const noexcept;

    VkResult CreateImage(const VkImageCreateInfo& ci, Image&) const noexcept;

    VkResult CreateImageView(const VkImageViewCreateInfo& ci, ImageView&) const noexcept;

    VkResult CreateFramebuffer(const VkFramebufferCreateInfo& ci, Framebuffer&) const noexcept;

    VkResult CreateFence(const VkFenceCreateInfo& ci, Fence&) const noexcept;

    VkResult CreateSampler(const VkSamplerCreateInfo& ci, Sampler&) const noexcept;

    VkResult WaitIdle() const noexcept { return dld->vkDeviceWaitIdle(handle); }

    VkResult AcquireNextImageKHR(VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                 VkFence fence, uint32_t* image_index) const noexcept {
        return dld->vkAcquireNextImageKHR(
            handle, swapchain, timeout, semaphore, fence, image_index);
    }

    const DeviceDispatch& Dispatch() const noexcept { return *dld; }

private:
};

class CommandBuffer : public Handle<VkCommandBuffer, NoOwnerLife, DeviceDispatch> {
    using Handle<VkCommandBuffer, NoOwnerLife, DeviceDispatch>::Handle;

public:
    VkResult Begin(const VkCommandBufferBeginInfo& begin_info) const {
        return dld->vkBeginCommandBuffer(handle, &begin_info);
    }

    VkResult End() const { return dld->vkEndCommandBuffer(handle); }

    void BeginRenderPass(const VkRenderPassBeginInfo& renderpass_bi,
                         VkSubpassContents            contents) const noexcept {
        dld->vkCmdBeginRenderPass(handle, &renderpass_bi, contents);
    }

    void EndRenderPass() const noexcept { dld->vkCmdEndRenderPass(handle); }

    void BeginQuery(VkQueryPool query_pool, uint32_t query,
                    VkQueryControlFlags flags) const noexcept {
        dld->vkCmdBeginQuery(handle, query_pool, query, flags);
    }

    void EndQuery(VkQueryPool query_pool, uint32_t query) const noexcept {
        dld->vkCmdEndQuery(handle, query_pool, query);
    }

    void BindDescriptorSets(VkPipelineBindPoint bind_point, VkPipelineLayout layout, uint32_t first,
                            Span<VkDescriptorSet> sets,
                            Span<uint32_t>        dynamic_offsets) const noexcept {
        dld->vkCmdBindDescriptorSets(handle,
                                     bind_point,
                                     layout,
                                     first,
                                     sets.size(),
                                     sets.data(),
                                     dynamic_offsets.size(),
                                     dynamic_offsets.data());
    }

    void PushDescriptorSetKHR(VkPipelineBindPoint bind_point, VkPipelineLayout layout, uint32_t set,
                              Span<const VkWriteDescriptorSet> wsets) const noexcept {
        assert(wsets[0].sType == VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
        dld->vkCmdPushDescriptorSetKHR(handle, bind_point, layout, set, wsets.size(), wsets.data());
    }

    void PushDescriptorSetWithTemplateKHR(VkDescriptorUpdateTemplateKHR update_template,
                                          VkPipelineLayout layout, uint32_t set,
                                          const void* data) const noexcept {
        dld->vkCmdPushDescriptorSetWithTemplateKHR(handle, update_template, layout, set, data);
    }

    void BindPipeline(VkPipelineBindPoint bind_point, VkPipeline pipeline) const noexcept {
        dld->vkCmdBindPipeline(handle, bind_point, pipeline);
    }

    void BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset,
                         VkIndexType index_type) const noexcept {
        dld->vkCmdBindIndexBuffer(handle, buffer, offset, index_type);
    }

    void BindVertexBuffers(uint32_t first, uint32_t count, const VkBuffer* buffers,
                           const VkDeviceSize* offsets) const noexcept {
        dld->vkCmdBindVertexBuffers(handle, first, count, buffers, offsets);
    }

    void BindVertexBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset) const noexcept {
        BindVertexBuffers(binding, 1, &buffer, &offset);
    }

    void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex,
              uint32_t first_instance) const noexcept {
        dld->vkCmdDraw(handle, vertex_count, instance_count, first_vertex, first_instance);
    }

    void DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index,
                     int32_t vertex_offset, uint32_t first_instance) const noexcept {
        dld->vkCmdDrawIndexed(
            handle, index_count, instance_count, first_index, vertex_offset, first_instance);
    }

    void ClearColorImage(VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor,
                         Span<const VkImageSubresourceRange> ranges) const noexcept {
        return dld->vkCmdClearColorImage(
            handle, image, imageLayout, pColor, ranges.size(), ranges.data());
    }

    void ClearAttachments(Span<VkClearAttachment> attachments,
                          Span<VkClearRect>       rects) const noexcept {
        dld->vkCmdClearAttachments(
            handle, attachments.size(), attachments.data(), rects.size(), rects.data());
    }

    void BlitImage(VkImage src_image, VkImageLayout src_layout, VkImage dst_image,
                   VkImageLayout dst_layout, Span<VkImageBlit> regions,
                   VkFilter filter) const noexcept {
        dld->vkCmdBlitImage(handle,
                            src_image,
                            src_layout,
                            dst_image,
                            dst_layout,
                            regions.size(),
                            regions.data(),
                            filter);
    }

    void ResolveImage(VkImage src_image, VkImageLayout src_layout, VkImage dst_image,
                      VkImageLayout dst_layout, Span<VkImageResolve> regions) {
        dld->vkCmdResolveImage(
            handle, src_image, src_layout, dst_image, dst_layout, regions.size(), regions.data());
    }

    void Dispatch(uint32_t x, uint32_t y, uint32_t z) const noexcept {
        dld->vkCmdDispatch(handle, x, y, z);
    }

    void PipelineBarrier(VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                         VkDependencyFlags                 dependency_flags,
                         Span<const VkMemoryBarrier>       memory_barriers,
                         Span<const VkBufferMemoryBarrier> buffer_barriers,
                         Span<const VkImageMemoryBarrier>  image_barriers) const noexcept {
        dld->vkCmdPipelineBarrier(handle,
                                  src_stage_mask,
                                  dst_stage_mask,
                                  dependency_flags,
                                  memory_barriers.size(),
                                  memory_barriers.data(),
                                  buffer_barriers.size(),
                                  buffer_barriers.data(),
                                  image_barriers.size(),
                                  image_barriers.data());
    }

    void PipelineBarrier(VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                         VkDependencyFlags dependency_flags = 0) const noexcept {
        PipelineBarrier(src_stage_mask, dst_stage_mask, dependency_flags, {}, {}, {});
    }

    void PipelineBarrier(VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                         VkDependencyFlags      dependency_flags,
                         const VkMemoryBarrier& memory_barrier) const noexcept {
        PipelineBarrier(src_stage_mask, dst_stage_mask, dependency_flags, memory_barrier, {}, {});
    }

    void PipelineBarrier(VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                         VkDependencyFlags            dependency_flags,
                         const VkBufferMemoryBarrier& buffer_barrier) const noexcept {
        PipelineBarrier(src_stage_mask, dst_stage_mask, dependency_flags, {}, buffer_barrier, {});
    }

    void PipelineBarrier(VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                         VkDependencyFlags           dependency_flags,
                         const VkImageMemoryBarrier& image_barrier) const noexcept {
        PipelineBarrier(src_stage_mask, dst_stage_mask, dependency_flags, {}, {}, image_barrier);
    }

    void CopyBufferToImage(VkBuffer src_buffer, VkImage dst_image, VkImageLayout dst_image_layout,
                           Span<VkBufferImageCopy> regions) const noexcept {
        dld->vkCmdCopyBufferToImage(
            handle, src_buffer, dst_image, dst_image_layout, regions.size(), regions.data());
    }

    void CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer,
                    Span<VkBufferCopy> regions) const noexcept {
        dld->vkCmdCopyBuffer(handle, src_buffer, dst_buffer, regions.size(), regions.data());
    }

    void CopyImage(VkImage src_image, VkImageLayout src_layout, VkImage dst_image,
                   VkImageLayout dst_layout, Span<VkImageCopy> regions) const noexcept {
        dld->vkCmdCopyImage(
            handle, src_image, src_layout, dst_image, dst_layout, regions.size(), regions.data());
    }

    void CopyImageToBuffer(VkImage src_image, VkImageLayout src_layout, VkBuffer dst_buffer,
                           Span<VkBufferImageCopy> regions) const noexcept {
        dld->vkCmdCopyImageToBuffer(
            handle, src_image, src_layout, dst_buffer, regions.size(), regions.data());
    }

    void FillBuffer(VkBuffer dst_buffer, VkDeviceSize dst_offset, VkDeviceSize size,
                    uint32_t data) const noexcept {
        dld->vkCmdFillBuffer(handle, dst_buffer, dst_offset, size, data);
    }

    void PushConstants(VkPipelineLayout layout, VkShaderStageFlags flags, uint32_t offset,
                       uint32_t size, const void* values) const noexcept {
        dld->vkCmdPushConstants(handle, layout, flags, offset, size, values);
    }

    template<typename T>
    void PushConstants(VkPipelineLayout layout, VkShaderStageFlags flags,
                       const T& data) const noexcept {
        static_assert(std::is_trivially_copyable_v<T>, "<data> is not trivially copyable");
        dld->vkCmdPushConstants(handle, layout, flags, 0, static_cast<uint32_t>(sizeof(T)), &data);
    }

    void SetViewport(uint32_t first, Span<VkViewport> viewports) const noexcept {
        dld->vkCmdSetViewport(handle, first, viewports.size(), viewports.data());
    }

    void SetScissor(uint32_t first, Span<VkRect2D> scissors) const noexcept {
        dld->vkCmdSetScissor(handle, first, scissors.size(), scissors.data());
    }

    void SetBlendConstants(const float blend_constants[4]) const noexcept {
        dld->vkCmdSetBlendConstants(handle, blend_constants);
    }

    void SetStencilCompareMask(VkStencilFaceFlags face_mask, uint32_t compare_mask) const noexcept {
        dld->vkCmdSetStencilCompareMask(handle, face_mask, compare_mask);
    }

    void SetStencilReference(VkStencilFaceFlags face_mask, uint32_t reference) const noexcept {
        dld->vkCmdSetStencilReference(handle, face_mask, reference);
    }

    void SetStencilWriteMask(VkStencilFaceFlags face_mask, uint32_t write_mask) const noexcept {
        dld->vkCmdSetStencilWriteMask(handle, face_mask, write_mask);
    }

    void SetDepthBias(float constant_factor, float clamp, float slope_factor) const noexcept {
        dld->vkCmdSetDepthBias(handle, constant_factor, clamp, slope_factor);
    }

    void SetDepthBounds(float min_depth_bounds, float max_depth_bounds) const noexcept {
        dld->vkCmdSetDepthBounds(handle, min_depth_bounds, max_depth_bounds);
    }

    void SetEvent(VkEvent event, VkPipelineStageFlags stage_flags) const noexcept {
        dld->vkCmdSetEvent(handle, event, stage_flags);
    }

    void WaitEvents(Span<VkEvent> events, VkPipelineStageFlags src_stage_mask,
                    VkPipelineStageFlags dst_stage_mask, Span<VkMemoryBarrier> memory_barriers,
                    Span<VkBufferMemoryBarrier> buffer_barriers,
                    Span<VkImageMemoryBarrier>  image_barriers) const noexcept {
        dld->vkCmdWaitEvents(handle,
                             events.size(),
                             events.data(),
                             src_stage_mask,
                             dst_stage_mask,
                             memory_barriers.size(),
                             memory_barriers.data(),
                             buffer_barriers.size(),
                             buffer_barriers.data(),
                             image_barriers.size(),
                             image_barriers.data());
    }

    void BindVertexBuffers2EXT(uint32_t first_binding, uint32_t binding_count,
                               const VkBuffer* buffers, const VkDeviceSize* offsets,
                               const VkDeviceSize* sizes,
                               const VkDeviceSize* strides) const noexcept {
        dld->vkCmdBindVertexBuffers2EXT(
            handle, first_binding, binding_count, buffers, offsets, sizes, strides);
    }

    void SetCullModeEXT(VkCullModeFlags cull_mode) const noexcept {
        dld->vkCmdSetCullModeEXT(handle, cull_mode);
    }

    void SetDepthBoundsTestEnableEXT(bool enable) const noexcept {
        dld->vkCmdSetDepthBoundsTestEnableEXT(handle, enable ? VK_TRUE : VK_FALSE);
    }

    void SetDepthCompareOpEXT(VkCompareOp compare_op) const noexcept {
        dld->vkCmdSetDepthCompareOpEXT(handle, compare_op);
    }

    void SetDepthTestEnableEXT(bool enable) const noexcept {
        dld->vkCmdSetDepthTestEnableEXT(handle, enable ? VK_TRUE : VK_FALSE);
    }

    void SetDepthWriteEnableEXT(bool enable) const noexcept {
        dld->vkCmdSetDepthWriteEnableEXT(handle, enable ? VK_TRUE : VK_FALSE);
    }

    void SetFrontFaceEXT(VkFrontFace front_face) const noexcept {
        dld->vkCmdSetFrontFaceEXT(handle, front_face);
    }

    void SetLineWidth(float line_width) const noexcept {
        dld->vkCmdSetLineWidth(handle, line_width);
    }

    void SetPrimitiveTopologyEXT(VkPrimitiveTopology primitive_topology) const noexcept {
        dld->vkCmdSetPrimitiveTopologyEXT(handle, primitive_topology);
    }

    void SetStencilOpEXT(VkStencilFaceFlags face_mask, VkStencilOp fail_op, VkStencilOp pass_op,
                         VkStencilOp depth_fail_op, VkCompareOp compare_op) const noexcept {
        dld->vkCmdSetStencilOpEXT(handle, face_mask, fail_op, pass_op, depth_fail_op, compare_op);
    }

    void SetStencilTestEnableEXT(bool enable) const noexcept {
        dld->vkCmdSetStencilTestEnableEXT(handle, enable ? VK_TRUE : VK_FALSE);
    }

    void SetVertexInputEXT(Span<VkVertexInputBindingDescription2EXT>   bindings,
                           Span<VkVertexInputAttributeDescription2EXT> attributes) const noexcept {
        dld->vkCmdSetVertexInputEXT(
            handle, bindings.size(), bindings.data(), attributes.size(), attributes.data());
    }

    void BindTransformFeedbackBuffersEXT(uint32_t first, uint32_t count, const VkBuffer* buffers,
                                         const VkDeviceSize* offsets,
                                         const VkDeviceSize* sizes) const noexcept {
        dld->vkCmdBindTransformFeedbackBuffersEXT(handle, first, count, buffers, offsets, sizes);
    }

    void BeginTransformFeedbackEXT(uint32_t first_counter_buffer, uint32_t counter_buffers_count,
                                   const VkBuffer*     counter_buffers,
                                   const VkDeviceSize* counter_buffer_offsets) const noexcept {
        dld->vkCmdBeginTransformFeedbackEXT(handle,
                                            first_counter_buffer,
                                            counter_buffers_count,
                                            counter_buffers,
                                            counter_buffer_offsets);
    }

    void EndTransformFeedbackEXT(uint32_t first_counter_buffer, uint32_t counter_buffers_count,
                                 const VkBuffer*     counter_buffers,
                                 const VkDeviceSize* counter_buffer_offsets) const noexcept {
        dld->vkCmdEndTransformFeedbackEXT(handle,
                                          first_counter_buffer,
                                          counter_buffers_count,
                                          counter_buffers,
                                          counter_buffer_offsets);
    }

    void BeginDebugUtilsLabelEXT(const char* label, std::span<float, 4> color) const noexcept {
        const VkDebugUtilsLabelEXT label_info {
            .sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
            .pNext      = nullptr,
            .pLabelName = label,
            .color { color[0], color[1], color[2], color[3] },
        };
        dld->vkCmdBeginDebugUtilsLabelEXT(handle, &label_info);
    }

    void EndDebugUtilsLabelEXT() const noexcept { dld->vkCmdEndDebugUtilsLabelEXT(handle); }
};

std::optional<std::vector<VkExtensionProperties>>
EnumerateInstanceExtensionProperties(const InstanceDispatch& dld);

std::optional<std::vector<VkLayerProperties>>
EnumerateInstanceLayerProperties(const InstanceDispatch& dld);
} // namespace vvk
