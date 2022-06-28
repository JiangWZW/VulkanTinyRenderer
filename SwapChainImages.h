#pragma once

#include "pch.h"

struct SwapChainImagesCreateInfo
{
	VkDevice device;
	VkSwapchainKHR swapchain;
	VkFormat surface_format;
};

class SwapChainImages
{
public:
	void Create(
		VkDevice                     device, 
		VkSwapchainKHR               swapchain, 
		VkFormat                     surface_format
	);

	void Destroy(
		VkDevice device
	);

	const std::vector<VkImageView>& GetImageViews() const;
	const std::vector<VkImage>& GetImages() const;
	int ImageCount() const { return images_.size(); }

private:
	std::vector<VkImage>         images_;
	std::vector<VkImageView>     img_views_;
};
