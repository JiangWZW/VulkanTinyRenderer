#include "pch.h"
#include "SwapChainFramebuffers.h"

/**
 * \brief Creates framebuffers for current swapchain in device
 * \param device 
 * \param renderpass Render pass that frame buffers bind to 
 * \param img_count 
 * \param img_views 
 * \param img_extents Decides the extent of images in framebuffer
 */
void SwapChainFramebuffers::Create(
	VkDevice device,
	VkRenderPass renderpass,
	const int img_count,
	const VkImageView* const img_views,
	const VkExtent2D* const img_extents
)
{
	data_.reserve(img_count);
	
	VkFramebufferCreateInfo framebuffer_ci = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	framebuffer_ci.renderPass = renderpass;
	// Each swap chain image is restricted with exactly 1 framebuffer
	framebuffer_ci.attachmentCount = 1;

	for (int i = 0; i < img_count; ++i)
	{
		framebuffer_ci.pAttachments = &(img_views[i]);
		framebuffer_ci.width = img_extents[i].width;
		framebuffer_ci.height = img_extents[i].height;
		framebuffer_ci.layers = 1;

		VkFramebuffer framebuffer;
		VK_CHECK(
			vkCreateFramebuffer(device, &framebuffer_ci, nullptr, &framebuffer),
			"Failed to create frame buffer"
		);
		data_.emplace_back(framebuffer);
	}
}

void SwapChainFramebuffers::Destroy(VkDevice device)
{
	for (const VkFramebuffer& framebuffer : data_)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
}

