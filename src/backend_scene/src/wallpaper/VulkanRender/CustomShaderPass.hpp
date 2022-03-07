#pragma once
#include "VulkanPass.hpp"
#include <string>
#include <vector>

#include "Vulkan/Device.hpp"
#include "Scene/Scene.h"
#include "Vulkan/StagingBuffer.hpp"
#include "Vulkan/GraphicsPipeline.hpp"

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
        std::vector<int> vk_tex_binding;
        ImageParameters vk_output;
        std::vector<StagingBufferRef> vertex_bufs;
        StagingBufferRef ubo_buf;

        bool blending {false};

        vk::Framebuffer fb;
        PipelineParameters pipeline;

        std::function<void()> update_uniform_op;
    };

    CustomShaderPass(const Desc&);
    virtual ~CustomShaderPass();

    void prepare(Scene&, const Device&, RenderingResources&) override;
    void execute(const Device&, RenderingResources&) override;
    void destory(const Device&, RenderingResources&) override;

private:
    Desc m_desc;
};

}
}