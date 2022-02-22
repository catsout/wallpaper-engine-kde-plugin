#pragma once
#include "VulkanPass.hpp"
#include <string>

namespace wallpaper
{
namespace vulkan
{

class CopyPass : public VulkanPass {
public:
    struct Desc {
        std::string src;
        std::string dst;
    };

    CopyPass(const Desc&);
private:
    Desc m_desc;
};

}
}