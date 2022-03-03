#pragma once
#include "Utils/NoCopyMove.hpp"
#include "Utils/span.hpp"
#include "Instance.hpp"

namespace wallpaper
{
namespace vulkan
{

class Device;

struct StagingBufferRef {
    vk::Buffer buffer;
    vk::DeviceSize offset;
    Span<uint8_t> mapped_span;
};

class StagingBufferPool : NoCopy,NoMove {
public:
    StagingBufferPool(const Device&);
    ~StagingBufferPool();
private:
    const Device& m_device;
};

}  
}
