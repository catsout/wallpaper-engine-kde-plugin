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
    virtual ~CopyPass();

    void prepare(Scene&, const Device&, RenderingResources&) override {};
    void execute(const Device&, RenderingResources&) override {};
    void destory(const Device&) override {}

private:
    Desc m_desc;
};

}
}