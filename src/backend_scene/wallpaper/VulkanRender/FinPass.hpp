#pragma once
#include "VulkanPass.hpp"
#include <string>

#include "Vulkan/Device.hpp"
#include "Vulkan/StagingBuffer.hpp"
#include "Vulkan/GraphicsPipeline.hpp"

#include "Scene/Scene.h"
#include "SpecTexs.h"

namespace wallpaper
{
namespace vulkan
{

class FinPass : public VulkanPass {
public:
    struct Desc {
        // in
        const std::string_view result {SpecTex_Default};
        vk::Format present_format;
        vk::ImageLayout present_layout;
        uint32_t present_queue_index;

        // prepared
        ImageParameters vk_result;
        ImageParameters vk_present;
        vk::ImageLayout render_layout;
        vk::ClearValue clear_value;

        StagingBufferRef vertex_buf; 
        vk::Framebuffer fb;
        PipelineParameters pipeline;
    };

    FinPass(const Desc&);
    virtual ~FinPass();

    void setPresent(ImageParameters);
    void setPresentLayout(vk::ImageLayout);
    void setPresentFormat(vk::Format);
    void setPresentQueueIndex(uint32_t);

    void prepare(Scene&, const Device&, RenderingResources&) override;
    void execute(const Device&, RenderingResources&) override;
    void destory(const Device&, RenderingResources&) override;

private:
    Desc m_desc;
};

}
}