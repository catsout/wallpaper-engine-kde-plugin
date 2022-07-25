#pragma once

#include "Swapchain/ExSwapchain.hpp"
#include "Device.hpp"
#include <cstdio>

namespace wallpaper
{
namespace vulkan
{

struct VulkanExHandle : NoCopy {
    ExHandle          handle;
    ExImageParameters image;

    VulkanExHandle()  = default;
    ~VulkanExHandle() = default;
    VulkanExHandle(VulkanExHandle&& o) noexcept: handle(o.handle), image(std::move(o.image)) {}
    VulkanExHandle& operator=(VulkanExHandle&& o) noexcept {
        handle = o.handle;
        image  = std::move(o.image);
        return *this;
    }
};
struct VulkanExHandleSemaphore {
    ExHandle    handle;
    VkSemaphore semaphore;
};

class VulkanExSwapchain : public ExSwapchain {
    using atomic_ = std::atomic<ExHandle*>;

public:
    VulkanExSwapchain(std::array<VulkanExHandle, 3> handles, VkExtent2D ext)
        : m_handles(std::move(handles)), m_extent(ext) {
        int index = 0;
        for (auto& h : m_handles) {
            auto& handle  = h.handle;
            handle        = ExHandle(index++);
            handle.width  = (i32)h.image.extent.width;
            handle.height = (i32)h.image.extent.height;
            handle.fd     = h.image.fd;
            handle.size   = h.image.mem_reqs.size;
        }
        m_presented  = &m_handles[0].handle;
        m_ready      = &m_handles[1].handle;
        m_inprogress = &m_handles[2].handle;
    }
    virtual ~VulkanExSwapchain() = default;

    uint width() const override { return m_extent.width; }
    uint height() const override { return m_extent.height; }

    const auto& handles() const { return m_handles; }

    ExImageParameters& GetInprogressImage() {
        return m_handles.at((usize)(*inprogress()).id()).image;
    }

    constexpr VkFormat format() const { return VK_FORMAT_R8G8B8A8_UNORM; };

protected:
    atomic_& presented() override { return m_presented; };
    atomic_& ready() override { return m_ready; };
    atomic_& inprogress() override { return m_inprogress; };

private:
    std::array<VulkanExHandle, 3> m_handles;
    atomic_                       m_presented { nullptr };
    atomic_                       m_ready { nullptr };
    atomic_                       m_inprogress { nullptr };
    VkExtent2D                    m_extent;
};

inline std::unique_ptr<VulkanExSwapchain> CreateExSwapchain(const Device& device, uint w, uint h,
                                                            VkImageTiling tiling) {
    std::array<VulkanExHandle, 3> handles;
    for (auto& handle : handles) {
        if (auto rv = device.tex_cache().CreateExTex(w, h, VK_FORMAT_R8G8B8A8_UNORM, tiling);
            rv.has_value())
            handle.image = std::move(rv.value());
        else
            return nullptr;
    }
    /*
    VulkanExHandleSemaphore handle_sem;
    {
        vk::SemaphoreCreateInfo info;
        vk::ExportSemaphoreCreateInfo esci { vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueFd };
        info.setPNext(&esci);
        VK_CHECK_RESULT_ACT(return nullptr, device.handle().createSemaphore(&info, nullptr,
    &handle_sem.semaphore)); vk::SemaphoreGetFdInfoKHR fd_info; fd_info.semaphore =
    handle_sem.semaphore; fd_info.handleType = vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueFd;
        VK_CHECK_RESULT_ACT(return nullptr, device.handle().getSemaphoreFdKHR(&fd_info,
    &handle_sem.handle.fd));
    }*/
    return std::make_unique<VulkanExSwapchain>(std::move(handles), VkExtent2D { w, h });
}

} // namespace vulkan
} // namespace wallpaper
