#pragma once
#include "Type.hpp"

#include <vector>
#include <memory>

namespace wallpaper
{
namespace vulkan
{

struct ShaderSpv {
    std::string entry_point { "main" };
    ShaderType  stage;

    std::vector<unsigned int> spirv;
};

using Uni_ShaderSpv = std::unique_ptr<ShaderSpv>;

} // namespace vulkan
} // namespace wallpaper
