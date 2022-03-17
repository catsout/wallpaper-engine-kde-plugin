#pragma once
#include "Utils/NoCopyMove.hpp"
#include "Vulkan/StagingBuffer.hpp"
#include <memory>

namespace wallpaper
{
namespace vulkan
{

struct RenderingResources {
    vk::CommandBuffer command;

	vk::Semaphore sem_swap_wait_image;
	vk::Semaphore sem_swap_finish;
	vk::Fence fence_frame;

	StagingBuffer* vertex_buf;
	StagingBuffer* dyn_buf;
};
}
}