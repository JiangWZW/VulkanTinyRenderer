#include "pch.h"
#include "SwapChainConfigurator.h"



void SwapChainConfigurator::FetchSwapChainConfigs(
	const VkPhysicalDevice& physical_device,
	const VkSurfaceKHR& surface)
{
	attribs_ = QuerySwapChainAttributes(physical_device, surface);
}

VkSurfaceCapabilitiesKHR SwapChainConfigurator::GetSurfaceCapabilities()
{
	return attribs_.surface_capabilities;
}


VkPresentModeKHR SwapChainConfigurator::GetPresentationMode()
{
	// Look for mail-box presentation mode
	for (const VkPresentModeKHR& present_mode : attribs_.presentation_modes)
	{
		// Triple Buffer Schema
		if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return present_mode;
		}
	}

	// Fall back, this always available, according to vulkan spec
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR SwapChainConfigurator::GetSurfaceFormat()
{
	if (AllSurfaceFormatsSupported())
	{
		return { default_surface_format_[0], default_surface_color_space_ };
	}

	// If not all supported, then try find proper one
	for (const auto &format : attribs_.surface_formats)
	{
		if (IsDefaultSurfaceFormat(format))
		{
			return format;
		}
	}

	// If can't find optimal format, then just return first format
	return attribs_.surface_formats[0];
}

uint32_t SwapChainConfigurator::GetMinImageCount() const
{
	// Get one more image than minimum image count to support triple-buffering
	// ("mailbox" presentation mode)
	uint32_t min_img_count = attribs_.surface_capabilities.minImageCount + 1;

	if (attribs_.surface_capabilities.maxImageCount > 0 
		// Not limitless(==0 means image count has no limit)
		&& min_img_count > attribs_.surface_capabilities.maxImageCount)
	{
		min_img_count = attribs_.surface_capabilities.maxImageCount;
	}

	return min_img_count;
}

VkExtent2D SwapChainConfigurator::GetSurfaceExtent(GLFWwindow *surface_window) const
{
	// currentExtent is the current width and height of the surface,
	if (attribs_.surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		// or the special value(0xFFFFFFFF, 0xFFFFFFFF),
		// Indicating that the surface size will be determined
		// by the extent of a swapchain targeting the surface.
		return attribs_.surface_capabilities.currentExtent;
	}

	// If value can vary(depends on current surface), set manually
	// Return frame buffer size
	int width, height;
	glfwGetFramebufferSize(surface_window, &width, &height);

	VkExtent2D extent;
	extent.width = static_cast<uint32_t>(width);
	extent.height = static_cast<uint32_t>(height);
	// Restrict extents
	extent.width = std::min(attribs_.surface_capabilities.maxImageExtent.width, extent.width);
	extent.width = std::max(attribs_.surface_capabilities.minImageExtent.width, extent.width);
	extent.height = std::min(attribs_.surface_capabilities.maxImageExtent.height, extent.height);
	extent.height = std::max(attribs_.surface_capabilities.minImageExtent.height, extent.height);

	return extent;
}

SwapChainAttributes SwapChainConfigurator::QuerySwapChainAttributes(
	const VkPhysicalDevice& physical_device,
	const VkSurfaceKHR& surface)
{
	SwapChainAttributes swapchain_attribs;

	// --- Capabilities ---
	uint32_t num_capabilities = 0;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		physical_device,
		surface,
		&(swapchain_attribs.surface_capabilities));

	// --- Surface Formats ---
	uint32_t num_formats = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &num_formats,nullptr);
	if (num_formats != 0)
	{
		swapchain_attribs.surface_formats.resize(num_formats);

		vkGetPhysicalDeviceSurfaceFormatsKHR(
			physical_device,
			surface,
			&num_formats,
			swapchain_attribs.surface_formats.data());
	}

	// --- Presentation Modes ---
	uint32_t num_present_modes = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &num_present_modes, nullptr);
	if (num_present_modes != 0)
	{
		swapchain_attribs.presentation_modes.resize(num_present_modes);

		vkGetPhysicalDeviceSurfacePresentModesKHR(
			physical_device,
			surface,
			&num_present_modes,
			swapchain_attribs.presentation_modes.data()
		);
	}

	return swapchain_attribs;
}

bool SwapChainConfigurator::AllSurfaceFormatsSupported()
{
	// If only 1 format is defined and is VK_FORMAT_UNDEFINED,
	// this means that all formats are supported.
	return (attribs_.surface_formats.size() == 1 &&
		attribs_.surface_formats[0].format == VK_FORMAT_UNDEFINED);
}

bool SwapChainConfigurator::IsDefaultSurfaceFormat(VkSurfaceFormatKHR format)
{
	return (format.format == default_surface_format_[0] || 
			format.format == default_surface_format_[1])
	&& (format.colorSpace == default_surface_color_space_);
}
