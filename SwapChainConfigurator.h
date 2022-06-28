#pragma once

#include "pch.h"

struct SwapChainAttributes
{
	VkSurfaceCapabilitiesKHR        surface_capabilities;
	std::vector<VkSurfaceFormatKHR> surface_formats;
	std::vector<VkPresentModeKHR>   presentation_modes;
};


class SwapChainConfigurator
{
public:
	static constexpr VkColorSpaceKHR default_surface_color_space_ = 
		VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	static constexpr VkFormat default_surface_format_[2] = 
	{
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_FORMAT_B8G8R8A8_UNORM
	};

	
	void FetchSwapChainConfigs(
		const VkPhysicalDevice& physical_device,
		const VkSurfaceKHR& surface
	);
	VkSurfaceCapabilitiesKHR GetSurfaceCapabilities();
	VkPresentModeKHR         GetPresentationMode();
	VkSurfaceFormatKHR       GetSurfaceFormat();
	VkExtent2D               GetSurfaceExtent(GLFWwindow* surface_window) const;
	uint32_t                 GetMinImageCount() const;

private:

	SwapChainAttributes attribs_;
	static SwapChainAttributes QuerySwapChainAttributes(
		const VkPhysicalDevice& physical_device, 
		const VkSurfaceKHR& surface
	);
	

	// Utilities
	bool AllSurfaceFormatsSupported();
	static bool IsDefaultSurfaceFormat(VkSurfaceFormatKHR format);
};
