#include "pch.h"
#include "SwapChainSyncPrimitives.h"

void SwapChainSyncPrimitives::Create(VkDevice device, int swapchain_img_count)
{
	// -- Semaphores --
	VkSemaphoreCreateInfo semaphore_ci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

	for (std::vector<VkSemaphore>& semaphore_list : semaphores_)
	{
		semaphore_list.resize(swapchain_img_count);
		for (VkSemaphore& semaphore : semaphore_list)
		{
			VK_CHECK(vkCreateSemaphore(device, &semaphore_ci, nullptr, &semaphore),
				"Failed to create semaphore");
		}
	}

	// -- Fences --
	VkFenceCreateInfo fence_ci = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	fences_.resize(swapchain_img_count);
	for (VkFence& fence : fences_)
	{
		VK_CHECK(vkCreateFence(device, &fence_ci, nullptr, &fence),
			"Failed to create fence");
	}
}

void SwapChainSyncPrimitives::Destroy(VkDevice device)
{
	for (std::vector<VkSemaphore>& semaphore_list : semaphores_)
	{
		for (VkSemaphore& semaphore : semaphore_list)
		{
			vkDestroySemaphore(device, semaphore, nullptr);
		}
	}

	for (VkFence& fence : fences_)
	{
		vkDestroyFence(device, fence, nullptr);
	}
}
