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

struct StagingBufferRef {
    vk::DeviceSize size {0};
    vk::DeviceSize offset {0};
    VmaVirtualAllocation allocation {};

    operator bool() const {
        return allocation != VK_NULL_HANDLE;
    }
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
    vk::Result mapStageBuf();
    const Device& m_device;
    vk::DeviceSize m_size;

    vk::BufferUsageFlags m_usage;

    void* m_stage_raw {nullptr};
    VmaVirtualBlock m_virtual_block {};

    BufferParameters m_stage_buf;
    BufferParameters m_gpu_buf;
};

}  
}
