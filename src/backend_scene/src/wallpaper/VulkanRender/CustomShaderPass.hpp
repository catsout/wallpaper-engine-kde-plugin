#pragma once
#include "VulkanPass.hpp"
#include <string>
#include <vector>

namespace wallpaper
{
namespace vulkan
{

class CustomShaderPass : public VulkanPass {
public:
    struct Desc {
        std::vector<std::string> textures;
    };

    CustomShaderPass(const Desc&);
private:
    Desc m_desc;
};

}
}