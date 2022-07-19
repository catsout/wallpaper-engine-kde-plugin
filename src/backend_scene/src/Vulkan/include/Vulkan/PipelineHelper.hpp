#pragma once
#include "Device.hpp"
#include "vvk/vulkan_wrapper.hpp"

namespace wallpaper
{
namespace vulkan
{
class DescriptorLayoutBuilder {
public:
    DescriptorLayoutBuilder(const Device& device): m_device(device) {}

    vvk::DescriptorSetLayout CreateDescriptorSetLayout(bool use_push_descriptor) const {
        if (m_bindings.empty()) {
            return {};
        }
        vk::DescriptorSetLayoutCreateInfo info;
        VkDescriptorSetLayoutCreateInfo   create_info {
              .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
              .pNext        = nullptr,
              .flags        = {},
              .bindingCount = (uint32_t)m_bindings.size(),
              .pBindings    = m_bindings.data(),
        };
        create_info.bindingCount = m_bindings.size();

        vvk::DescriptorSetLayout layout;
        VVK_CHECK(m_device.handle().CreateDescriptorSetLayout(create_info, layout));
        return layout;
    }

    vvk::PipelineLayout CreatePipelineLayout(vk::DescriptorSetLayout descriptor_set_layout) const {
        VkPipelineLayoutCreateInfo ci {
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext                  = nullptr,
            .setLayoutCount         = 1,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges    = nullptr
            //.pSetLayouts = descriptor_set_layout
        };
        vvk::PipelineLayout pipeline_layout;
        VVK_CHECK(m_device.handle().CreatePipelineLayout(ci, pipeline_layout));
        return pipeline_layout;
    }

private:
    const Device&                             m_device;
    std::vector<VkDescriptorSetLayoutBinding> m_bindings;
};

} // namespace vulkan
} // namespace wallpaper
