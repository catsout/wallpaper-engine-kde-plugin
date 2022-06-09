#pragma once
#include "Device.hpp"

namespace wallpaper
{
namespace vulkan
{
class DescriptorLayoutBuilder {
public:
    DescriptorLayoutBuilder(const Device& device) : m_device(device) {}

    vk::DescriptorSetLayout CreateDescriptorSetLayout(bool use_push_descriptor) const {
        if (m_bindings.empty()) {
            return {};
        }
        vk::DescriptorSetLayoutCreateInfo info;
        info.setFlags({})
            .setBindingCount(m_bindings.size())
            .setPBindings(m_bindings.data());
        return m_device.handle().createDescriptorSetLayout(info).value;
    }

    vk::PipelineLayout CreatePipelineLayout(vk::DescriptorSetLayout descriptor_set_layout) const {
        vk::PipelineLayoutCreateInfo info;
		//info.setPSetLayouts(descriptor_set_layout)
		info.setSetLayoutCount(1)
            .setPPushConstantRanges(nullptr)
            .setPushConstantRangeCount(0);
		return m_device.handle().createPipelineLayout(info).value;
    }

private:
    const Device& m_device;
    std::vector<vk::DescriptorSetLayoutBinding> m_bindings;
};

}
}