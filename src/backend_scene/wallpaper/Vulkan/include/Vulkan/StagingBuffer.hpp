#pragma once
#include "Utils/NoCopyMove.hpp"
#include "Utils/span.hpp"
#include "Instance.hpp"
#include "Parameters.hpp"
#include "vk_mem_alloc.h"

namespace wallpaper
{
namespace vulkan
{

class Device;
class StagingBuffer;

class StagingBufferRef {
public:
    vk::DeviceSize size {0};
    vk::DeviceSize offset {0};

    operator bool() const {
        return m_allocation != VK_NULL_HANDLE;
    }

private:
    friend class StagingBuffer;
    VmaVirtualAllocation m_allocation {};
    size_t m_virtual_index {0};
};

class StagingBuffer : NoCopy,NoMove {
public:
    StagingBuffer(const Device&, vk::DeviceSize size, vk::BufferUsageFlags);
    ~StagingBuffer();

    bool allocate();
    void destroy();

    bool allocateSubRef(vk::DeviceSize size, StagingBufferRef&, vk::DeviceSize alignment=1);
    void unallocateSubRef(const StagingBufferRef&);
    bool writeToBuf(const StagingBufferRef&, Span<uint8_t>, size_t offset=0);

    bool recordUpload(vk::CommandBuffer&);

    const vk::Buffer& gpuBuf() const;
private:
    struct VirtualBlock {
        VmaVirtualBlock handle  {};
        bool            enabled {false};
        size_t          index   {0};
        vk::DeviceSize  offset  {0};
        vk::DeviceSize  size    {0};
    };

    vk::Result mapStageBuf();
    VirtualBlock* newVirtualBlock(vk::DeviceSize);
    bool increaseBuf(vk::DeviceSize);


    const Device& m_device;
    vk::DeviceSize m_size_step;

    vk::BufferUsageFlags m_usage;

    void* m_stage_raw {nullptr};
    std::vector<VirtualBlock> m_virtual_blocks {};

    BufferParameters m_stage_buf;
    BufferParameters m_gpu_buf;
};

}  
}
