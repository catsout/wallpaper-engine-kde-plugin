#pragma once

#include "Instance.hpp"

namespace wallpaper
{
namespace vulkan
{

struct VertexInputState {
    VkPipelineInputAssemblyStateCreateInfo         input_assembly;
    VkPipelineVertexInputStateCreateInfo           input;
    std::vector<VkVertexInputBindingDescription>   bind_descriptions;
    std::vector<VkVertexInputAttributeDescription> attr_descriptions;
};

} // namespace vulkan
} // namespace wallpaper
