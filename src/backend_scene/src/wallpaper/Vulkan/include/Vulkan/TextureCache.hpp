#pragma once

#include "Device.hpp"

namespace wallpaper
{
namespace vulkan
{


class TextureCache {
public:
    TextureCache(const Device& device):m_device(device) {}
    vk::ResultValue<ExImageParameters> CreateExTex(uint32_t witdh, uint32_t height, vk::Format);
private:
    const Device& m_device;
};

}
}