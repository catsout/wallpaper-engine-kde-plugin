#pragma once
#include "Instance.hpp"
#include "Utils/span.hpp"

namespace wallpaper
{
namespace vulkan
{
class ImageParameters;
class Device;
class Swapchain {
public:
    static vk::Result Create(Device&, vk::SurfaceKHR, vk::Extent2D, Swapchain&);
    const vk::SwapchainKHR& handle() const;
    vk::Format format() const;
    vk::Extent2D extent() const;
    vk::PresentModeKHR presentMode() const;
    Span<ImageParameters> images() const;
private:
    vk::SwapchainKHR m_handle;
	vk::Format m_format;
	vk::Extent2D m_extent;
    vk::PresentModeKHR m_present_mode;
	std::vector<ImageParameters> m_images;
};
}   
}
