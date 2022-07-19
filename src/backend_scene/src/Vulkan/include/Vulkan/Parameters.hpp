#pragma once

#include "Instance.hpp"
#include "Swapchain.hpp"
#include "Utils/NoCopyMove.hpp"
#include "vk_mem_alloc.h"
#include "vvk/vma_wrapper.hpp"

namespace wallpaper
{
namespace vulkan
{

struct QueueParameters {
    vvk::Queue handle;
    uint32_t   family_index;
};

struct VmaBufferParameters {
    vvk::VmaBuffer handle;
    std::size_t    req_size;

    VmaBufferParameters();
    ~VmaBufferParameters();
    VmaBufferParameters(VmaBufferParameters&& o) noexcept;
    VmaBufferParameters& operator=(VmaBufferParameters&& o) noexcept;
};

struct BufferParameters {
    VkBuffer    handle;
    std::size_t req_size;
    BufferParameters()  = default;
    ~BufferParameters() = default;
    BufferParameters(const VmaBufferParameters& o) noexcept
        : handle(*o.handle), req_size(o.req_size) {}
};

struct VmaImageParameters : NoCopy {
    vvk::VmaImage  handle;
    vvk::ImageView view;
    vvk::Sampler   sampler;
    VkExtent3D     extent;
    uint           mipmap_level { 1 };

    VmaImageParameters();
    ~VmaImageParameters();
    VmaImageParameters(VmaImageParameters&& o) noexcept;
    VmaImageParameters& operator=(VmaImageParameters&& o) noexcept;
};

struct ExImageParameters : NoCopy {
    vvk::DeviceMemory    mem {};
    VkMemoryRequirements mem_reqs {};

    vvk::Image     handle;
    vvk::ImageView view;
    vvk::Sampler   sampler;
    VkExtent3D     extent;
    uint           mipmap_level { 1 };
    int            fd { 0 };

    ExImageParameters();
    ~ExImageParameters();
    ExImageParameters(ExImageParameters&& o) noexcept;
    ExImageParameters& operator=(ExImageParameters&& o) noexcept;
};

struct ImageParameters {
    VkImage     handle;
    VkImageView view;
    VkSampler   sampler;
    VkExtent3D  extent;
    uint        mipmap_level { 1 };

    ImageParameters()  = default;
    ~ImageParameters() = default;
    ImageParameters(const VmaImageParameters& o) noexcept
        : handle(*o.handle),
          view(*o.view),
          sampler(*o.sampler),
          extent(o.extent),
          mipmap_level(o.mipmap_level) {}
    ImageParameters(const ExImageParameters& o) noexcept
        : handle(*o.handle),
          view(*o.view),
          sampler(*o.sampler),
          extent(o.extent),
          mipmap_level(o.mipmap_level) {}
};

struct ImageSlots : NoCopy {
    std::vector<VmaImageParameters> slots;

    ImageSlots();
    ~ImageSlots();
    ImageSlots(ImageSlots&& o) noexcept;
    ImageSlots& operator=(ImageSlots&& o) noexcept;
};

struct ImageSlotsRef {
    std::vector<ImageParameters> slots;

    int active { 0 };

    auto& getActive() const {
        if (active >= slots.size()) return slots[0];
        return slots[active];
    }
    ImageSlotsRef();
    ~ImageSlotsRef();
    ImageSlotsRef(const ImageSlots&);
};

} // namespace vulkan
} // namespace wallpaper
