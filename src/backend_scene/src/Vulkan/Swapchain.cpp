#include "Swapchain.hpp"

#include "Device.hpp"

using namespace wallpaper::vulkan;

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

namespace
{

bool querySwapChainSupport(const vvk::PhysicalDevice& gpu, VkSurfaceKHR surface,
                           SwapChainSupportDetails& details) {
    VVK_CHECK_BOOL_RE(gpu.GetSurfaceCapabilitiesKHR(surface, details.capabilities));
    VVK_CHECK_BOOL_RE(gpu.GetSurfaceFormatsKHR(surface, details.formats));
    VVK_CHECK_BOOL_RE(gpu.GetSurfacePresentModesKHR(surface, details.presentModes));
    return true;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(Span<const VkSurfaceFormatKHR> availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM ||
            availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM) {
            if (availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;
        }
    }
    auto& format = availableFormats[0];
    LOG_INFO("swapchain format: %s, color space: %s",
             vvk::ToString(format.format),
             vvk::ToString(format.colorSpace));
    return format;
}

VkExtent2D GetSwapChainExtent(VkSurfaceCapabilitiesKHR& surface_capabilities, VkExtent2D ext) {
    if (surface_capabilities.currentExtent.width == 0) {
        auto min = surface_capabilities.minImageExtent;
        auto max = surface_capabilities.maxImageExtent;
        if (ext.width < min.width) {
            ext.width = min.width;
        }
        if (ext.height < min.height) {
            ext.height = min.height;
        }
        if (ext.width > max.width) {
            ext.width = max.width;
        }
        if (ext.height > max.height) {
            ext.height = max.height;
        }
        return ext;
    }

    // Most of the cases we define size of the swap_chain images equal to current window's size
    return surface_capabilities.currentExtent;
}

std::optional<vvk::ImageView> CreateSwapImageView(const vvk::Device& device, VkFormat format,
                                                  VkImage image) {
    VkImageViewCreateInfo ci {
        .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext    = nullptr,
        .image    = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format   = format,
        .subresourceRange =
            VkImageSubresourceRange {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1,
            },
    };
    vvk::ImageView view;
    if (auto res = device.CreateImageView(ci, view); res == VK_SUCCESS) {
        return view;
    }
    return std::nullopt;
}
} // namespace

const vvk::SwapchainKHR& Swapchain::handle() const { return m_handle; }
VkFormat                 Swapchain::format() const { return m_format.format; };
VkExtent2D               Swapchain::extent() const { return m_extent; };

Span<const ImageParameters> Swapchain::images() const { return m_images; }

VkPresentModeKHR Swapchain::presentMode() const { return m_present_mode; };

bool Swapchain::Create(Device& device, VkSurfaceKHR surface, VkExtent2D extent, Swapchain& swap) {
    SwapChainSupportDetails swap_details;
    if (! querySwapChainSupport(device.gpu(), surface, swap_details)) return false;

    swap.m_format = chooseSwapSurfaceFormat(swap_details.formats);

    auto& surfaceCapabilities = swap_details.capabilities;

    // triple
    uint32_t image_count = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && image_count > surfaceCapabilities.maxImageCount)
        image_count = surfaceCapabilities.maxImageCount;

    swap.m_extent = GetSwapChainExtent(surfaceCapabilities, extent);

    swap.m_present_mode                          = VK_PRESENT_MODE_FIFO_KHR;
    VkSurfaceTransformFlagBitsKHR preTransform   = surfaceCapabilities.currentTransform;
    VkCompositeAlphaFlagBitsKHR   compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    VkSwapchainCreateInfoKHR sci {
        .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext            = nullptr,
        .surface          = surface,
        .minImageCount    = image_count,
        .imageFormat      = swap.m_format.format,
        .imageColorSpace  = swap.m_format.colorSpace,
        .imageExtent      = swap.m_extent,
        .imageArrayLayers = 1,
        .imageUsage       = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform     = preTransform,
        .compositeAlpha   = compositeAlpha,
        .presentMode      = swap.m_present_mode,
        .clipped          = true,
        .oldSwapchain     = nullptr,
    };

    VVK_CHECK_BOOL_RE(device.device().CreateSwapchainKHR(sci, swap.m_handle));
    {
        std::vector<VkImage> images;
        VVK_CHECK_BOOL_RE(swap.m_handle.GetImages(images));
        std::transform(
            images.begin(), images.end(), std::back_inserter(swap.m_images), [&](auto image) {
                ImageParameters image_paras {};
                image_paras.handle = image;
                image_paras.extent = { swap.m_extent.width, swap.m_extent.height, 1 };
                if (auto opt = CreateSwapImageView(device.device(), swap.m_format.format, image);
                    opt.has_value()) {
                    swap.m_imageviews.emplace_back(std::move(opt.value()));
                    image_paras.view = *swap.m_imageviews.back();
                }
                return image_paras;
            });
    }
    return true;
}
