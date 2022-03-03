#pragma once
#include "VulkanPass.hpp"
#include <string>
#include <vector>

#include "Vulkan/Device.hpp"
#include "Scene/Scene.h"

namespace wallpaper
{

namespace vulkan
{

class CustomShaderPass : public VulkanPass {
public:
    struct Desc {
        // in
        SceneNode* node {nullptr};
        std::vector<std::string> textures;
        std::string output;
        // prepared
        std::vector<ImageSlots> vk_textures;
        ImageParameters vk_output;
    };

    CustomShaderPass(const Desc&);
    virtual ~CustomShaderPass();

    void prepare(Scene&, const Device&, RenderingResources&) override;
    void execute(const Device&, RenderingResources&) override;
    void destory(const Device&) override;

private:
    Desc m_desc;
};

}
}