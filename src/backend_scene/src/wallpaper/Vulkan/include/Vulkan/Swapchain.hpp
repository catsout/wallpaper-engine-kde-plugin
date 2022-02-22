#pragma once
#include "Instance.hpp"

namespace wallpaper
{
namespace vulkan
{
class ImageParameters;
class Device;
class Swapchain {
public:
    static vk::Result Create(Device&, vk::SurfaceKHR, vk::Extent2D, Swapchain&);
    vk::SwapchainKHR& handle();
    vk::Format format() const;
    vk::Extent2D extent() const;
    vk::PresentModeKHR presentMode() const;
private:
    vk::SwapchainKHR m_handle;
	vk::Format m_format;
	vk::Extent2D m_extent;
    vk::PresentModeKHR m_present_mode;
	std::vector<ImageParameters> m_images;
};
}   
}
