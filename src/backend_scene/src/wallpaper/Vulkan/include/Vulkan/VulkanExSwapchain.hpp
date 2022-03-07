#pragma once

#include "Swapchain/ExSwapchain.hpp"
#include "TextureCache.hpp"
#include <cstdio>

namespace wallpaper
{
namespace vulkan
{

struct VulkanExHandle {
    ExHandle handle;
    ExImageParameters image;
};

class VulkanExSwapchain : public ExSwapchain {
    using atomic_ = std::atomic<ExHandle*>;
public:
    VulkanExSwapchain(std::array<VulkanExHandle, 3> handles):m_handles(handles) {
        int index = 0;
        for(auto& h:m_handles) {
            auto& handle = h.handle;
            handle = ExHandle(index++);
            handle.width = h.image.extent.width;
            handle.height = h.image.extent.height;
            handle.fd = h.image.fd;
            handle.size = h.image.mem_reqs.size;
        }
        m_presented = &m_handles[0].handle;
        m_ready = &m_handles[1].handle;
        m_inprogress = &m_handles[2].handle;
    }
    virtual ~VulkanExSwapchain() = default;

    ExImageParameters& GetInprogressImage() {
        return m_handles.at((*inprogress()).id()).image;
    }

    constexpr vk::Format format() const { return vk::Format::eR8G8B8A8Unorm; };
protected:
    atomic_& presented()  override { return m_presented; };
    atomic_& ready()      override { return m_ready; };
    atomic_& inprogress() override { return m_inprogress; };
private:
    std::array<VulkanExHandle, 3> m_handles;
    atomic_ m_presented  {nullptr};
    atomic_ m_ready      {nullptr};
    atomic_ m_inprogress {nullptr};
};

inline std::unique_ptr<VulkanExSwapchain> CreateExSwapchain(TextureCache& cache, std::uint32_t w, std::uint32_t h) {
    std::array<VulkanExHandle, 3> handles;
    for(auto& handle:handles) {
        auto rv = cache.CreateExTex(w, h, vk::Format::eR8G8B8A8Unorm);
        if(rv.result != vk::Result::eSuccess) return nullptr;
        handle.image = rv.value;
    }
    return std::make_unique<VulkanExSwapchain>(handles);
}

}
}