#pragma once
#include "Instance.hpp"
#include "Utils/span.hpp"

namespace wallpaper
{
namespace vulkan
{
class ImageParameters;
class VmaImageParameters;
class Device;
class Swapchain {
public:
    static bool                 Create(Device&, VkSurfaceKHR, VkExtent2D, Swapchain&);
    const vvk::SwapchainKHR&    handle() const;
    VkFormat                    format() const;
    VkExtent2D                  extent() const;
    VkPresentModeKHR            presentMode() const;
    Span<const ImageParameters> images() const;

private:
    vvk::SwapchainKHR            m_handle;
    VkSurfaceFormatKHR           m_format;
    VkExtent2D                   m_extent;
    VkPresentModeKHR             m_present_mode;
    std::vector<ImageParameters> m_images;
    std::vector<vvk::ImageView>  m_imageviews;
};
} // namespace vulkan
} // namespace wallpaper
