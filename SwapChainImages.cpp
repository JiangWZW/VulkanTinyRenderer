#include "pch.h"
#include "SwapChainImages.h"


void SwapChainImages::Create(
	VkDevice       device,
	VkSwapchainKHR swapchain,
	VkFormat       surface_format)
{
	images_ = std::vector<VkImage>(0);
	img_views_ = std::vector<VkImageView>(0);

	// Fetch VkImages from VkSwapChainKHR
	uint32_t img_count = 0;
	vkGetSwapchainImagesKHR(device, swapchain, &img_count, nullptr);
	images_.resize(img_count);
	img_views_.reserve(img_count);
	vkGetSwapchainImagesKHR(device, swapchain, &img_count, images_.data());

	// Create VkImageVIew for each VkImage in swap chain
	VkImageViewCreateInfo create_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	create_info.format = surface_format;
	create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	create_info.subresourceRange = {
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		1,
		0,
		1,
	};
	create_info.components = {
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY
	};
	
	for (const VkImage& vk_img : images_)
	{
		create_info.image = vk_img; // set image field

		VkImageView img_view;
		VkResult res = vkCreateImageView(device, &create_info, nullptr, &img_view);
		if (res != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to get swap chain image view");
		}
		img_views_.emplace_back(img_view);
	}
}


void SwapChainImages::Destroy(VkDevice device)
{
	for (VkImageView img_view : img_views_)
	{
		vkDestroyImageView(device, img_view, nullptr);
	}
}

const std::vector<VkImageView>& SwapChainImages::GetImageViews() const
{
	return img_views_;
}

const std::vector<VkImage>& SwapChainImages::GetImages() const
{
	return images_;
}
