#pragma once
#include "Instance.hpp"
#include "Parameters.hpp"
#include "vk_mem_alloc.h"

namespace wallpaper
{
namespace vulkan
{

bool CreateStagingBuffer(VmaAllocator, std::size_t size, BufferParameters& buffer);
}
}