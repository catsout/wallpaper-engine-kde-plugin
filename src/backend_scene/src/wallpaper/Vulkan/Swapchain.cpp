#include "Swapchain.hpp"

#include "Device.hpp"

using namespace wallpaper::vulkan;

vk::Format GetFormat(const vk::PhysicalDevice &phy, vk::SurfaceKHR &surface)
{
	std::vector<vk::SurfaceFormatKHR> formats = phy.getSurfaceFormatsKHR(surface);
	return (formats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : formats[0].format;
}

vk::Extent2D GetSwapChainExtent(vk::SurfaceCapabilitiesKHR &surface_capabilities, vk::Extent2D ext)
{
	if (surface_capabilities.currentExtent.width == -1)
	{
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

vk::ImageView CreateSwapImageView(const vk::Device &device, vk::Format format, vk::Image &image)
{
	vk::ImageViewCreateInfo createinfo;
	createinfo
		.setFormat(format)
		.setImage(image)
		.setViewType(vk::ImageViewType::e2D);
	createinfo.subresourceRange
		.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseMipLevel(0)
		.setLevelCount(1)
		.setBaseArrayLayer(0)
		.setLayerCount(1);
	return device.createImageView(createinfo);
}


vk::SwapchainKHR& Swapchain::handle() { return m_handle; }
vk::Format Swapchain::format() const { return m_format; };
vk::Extent2D Swapchain::extent() const { return m_extent; };

vk::PresentModeKHR Swapchain::presentMode() const { return m_present_mode; };

vk::Result Swapchain::Create(Device& device, vk::SurfaceKHR surface, vk::Extent2D extent, Swapchain& swap) {
	
	swap.m_format = GetFormat(device.gpu(), surface);

	vk::SurfaceCapabilitiesKHR surfaceCapabilities = device.gpu().getSurfaceCapabilitiesKHR(surface);

	// triple
	uint32_t image_count = surfaceCapabilities.minImageCount + 1;
	if (image_count > surfaceCapabilities.maxImageCount)
	{
		image_count = surfaceCapabilities.maxImageCount;
	}

	swap.m_extent = GetSwapChainExtent(surfaceCapabilities, extent);

	swap.m_present_mode = vk::PresentModeKHR::eFifo;
	vk::SurfaceTransformFlagBitsKHR preTransform =
		(surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
			? vk::SurfaceTransformFlagBitsKHR::eIdentity
			: surfaceCapabilities.currentTransform;

	vk::CompositeAlphaFlagBitsKHR compositeAlpha =
		(surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
			? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
		: (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
			? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
		: (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
			? vk::CompositeAlphaFlagBitsKHR::eInherit
			: vk::CompositeAlphaFlagBitsKHR::eOpaque;

	vk::SwapchainCreateInfoKHR swapCreateInfo;
	swapCreateInfo
		.setSurface(surface)
		.setMinImageCount(image_count)
		.setImageFormat(swap.m_format)
		.setImageExtent(swap.m_extent)
		.setPresentMode(swap.m_present_mode)
		.setPreTransform(preTransform)
		.setCompositeAlpha(compositeAlpha)
		.setImageSharingMode(vk::SharingMode::eExclusive)
		.setImageUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eColorAttachment)
		.setImageArrayLayers(1)
		.setClipped(true)
		.setOldSwapchain(nullptr);
	auto result = device.device().createSwapchainKHR(&swapCreateInfo, nullptr, &swap.m_handle);
	if(result != vk::Result::eSuccess) return result;
	{
		auto rv_images = device.device().getSwapchainImagesKHR(swap.m_handle);
		result = rv_images.result;
		auto& images = rv_images.value;
		if(result != vk::Result::eSuccess) return result;
		std::transform(images.begin(), images.end(), std::back_inserter(swap.m_images), [&](auto image)
					   { return ImageParameters{
							 .handle = image,
							 .view = CreateSwapImageView(device.device(), swap.m_format, image)}; });
	}
	return result;
}