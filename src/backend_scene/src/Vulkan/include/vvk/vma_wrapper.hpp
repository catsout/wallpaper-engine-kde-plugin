#pragma once

#include <cstddef>
#include <new>

#include "handle.hpp"
#include "vk_mem_alloc.h"
#include "vulkan_wrapper.hpp"

namespace vvk
{

struct VmaOwner {
    VmaAllocator      allocator {};
    VmaAllocation     allocation {};
    VmaAllocationInfo allocationInfo {};

    VmaOwner()  = default;
    ~VmaOwner() = default;

    VmaOwner(std::nullptr_t) {}
    VmaOwner& operator=(std::nullptr_t) {
        allocator      = {};
        allocation     = {};
        allocationInfo = {};
        return *this;
    }
};

inline void Destroy(VmaAllocator allocator, int) { vmaDestroyAllocator(allocator); }

inline void Destroy(VmaOwner owner, VkBuffer handle, int) {
    vmaDestroyBuffer(owner.allocator, handle, owner.allocation);
}
inline void Destroy(VmaOwner owner, VkImage handle, int) {
    vmaDestroyImage(owner.allocator, handle, owner.allocation);
}

class VmaAllocatorHandle : NoCopy {
public:
    VmaAllocatorHandle() = default;
    VmaAllocatorHandle(VmaAllocator vma, int): m_allocator(vma) {};
    ~VmaAllocatorHandle() { Release(); }

    VmaAllocatorHandle(VmaAllocatorHandle&& o)
        : m_allocator(std::exchange(o.m_allocator, nullptr)) {};
    VmaAllocatorHandle& operator=(VmaAllocatorHandle&& o) noexcept {
        Release();
        m_allocator = std::exchange(o.m_allocator, nullptr);
        return *this;
    };

    const auto& operator*() const noexcept { return m_allocator; }

private:
    void Release() {
        if (m_allocator) Destroy(m_allocator, 0);
    }
    VmaAllocator m_allocator {};
};

class VmaBuffer : public Handle<VkBuffer, VmaOwner, int> {
    using Handle<VkBuffer, VmaOwner, int>::Handle;

public:
    VmaAllocation Allocation() const noexcept { return owner.allocation; }

    VkResult MapMemory(void** data) const {
        return vmaMapMemory(owner.allocator, owner.allocation, data);
    }

    void UnMapMemory() { vmaUnmapMemory(owner.allocator, owner.allocation); }
};

class VmaImage : public Handle<VkImage, VmaOwner, int> {
    using Handle<VkImage, VmaOwner, int>::Handle;

public:
    VmaAllocation Allocation() const noexcept { return owner.allocation; }

    VkResult MapMemory(void** data) const {
        return vmaMapMemory(owner.allocator, owner.allocation, data);
    }

    void UnMapMemory() { vmaUnmapMemory(owner.allocator, owner.allocation); }
};

inline VkResult CreateVmaAllocator(const VmaAllocatorCreateInfo& ci,
                                   VmaAllocatorHandle&           allocator_h) noexcept {
    VmaAllocator object;

    auto res = vmaCreateAllocator(&ci, &object);
    if (res == VK_SUCCESS) allocator_h = VmaAllocatorHandle(object, 0);
    return res;
}

inline VkResult CreateBuffer(const VmaAllocator& vma_allocator, const VkBufferCreateInfo& ci,
                             const VmaAllocationCreateInfo& vma_info, VmaBuffer& buffer) noexcept {
    VkBuffer object;
    VmaOwner owner;
    owner.allocator = vma_allocator;

    auto res = vmaCreateBuffer(
        vma_allocator, &ci, &vma_info, &object, &owner.allocation, &owner.allocationInfo);
    if (res == VK_SUCCESS) buffer = VmaBuffer(object, owner, 0);
    return res;
}

inline VkResult CreateImage(const VmaAllocator& vma_allocator, const VkImageCreateInfo& ci,
                            const VmaAllocationCreateInfo& vma_info, VmaImage& vma_img) noexcept {
    VkImage  object;
    VmaOwner owner;
    owner.allocator = vma_allocator;

    auto res = vmaCreateImage(
        vma_allocator, &ci, &vma_info, &object, &owner.allocation, &owner.allocationInfo);
    if (res == VK_SUCCESS) vma_img = VmaImage(object, owner, 0);
    return res;
}

} // namespace vvk
