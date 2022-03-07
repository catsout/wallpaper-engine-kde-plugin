#pragma once
#include "Instance.hpp"

namespace wallpaper
{
namespace vulkan
{

struct ShaderSpv {
    std::string entry_point {"main"};
    vk::ShaderStageFlagBits stage;
    std::vector<unsigned int> spirv;
};

using Uni_ShaderSpv = std::unique_ptr<ShaderSpv>;

}
}