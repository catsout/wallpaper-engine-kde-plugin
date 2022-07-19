#pragma once
#include "Utils/NoCopyMove.hpp"
#include "Vulkan/StagingBuffer.hpp"
#include <memory>

namespace wallpaper
{
namespace vulkan
{

struct RenderingResources {
    vvk::CommandBuffer command;

    vvk::Semaphore sem_swap_wait_image;
    vvk::Semaphore sem_swap_finish;
    vvk::Fence     fence_frame;

    StagingBuffer* vertex_buf;
    StagingBuffer* dyn_buf;
};
} // namespace vulkan
} // namespace wallpaper
