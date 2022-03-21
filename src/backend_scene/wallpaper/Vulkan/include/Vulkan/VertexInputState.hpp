#pragma once

#include "Instance.hpp"

namespace wallpaper
{
namespace vulkan
{

struct VertexInputState {
    vk::PipelineInputAssemblyStateCreateInfo input_assembly;
    vk::PipelineVertexInputStateCreateInfo input;
    std::vector<vk::VertexInputBindingDescription> bind_descriptions;
    std::vector<vk::VertexInputAttributeDescription> attr_descriptions;
};

}
}