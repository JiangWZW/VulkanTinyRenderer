#pragma once

#include "pch.h"

class SwapChainSyncPrimitives
{
public:
	void Create(VkDevice device, int swapchain_img_count = 3);
	void Destroy(VkDevice device);

	const VkSemaphore& AcquireSemaphore(const int id = 0) { return semaphores_[0][id]; }
	const VkSemaphore& PresentSemaphore(const int id = 0) { return semaphores_[1][id]; }
	const VkFence& CmdBufferFence(const int id = 0) { return fences_[id]; }
	
private:
/**
 * \brief We only need 2 types of semaphore for now,
 * each type has multiple semaphores, for different frame ids
 */
#define NUM_SEMAPHORE_TYPES 2
	std::array<std::vector<VkSemaphore>, NUM_SEMAPHORE_TYPES>  semaphores_{};

	std::vector<VkFence> fences_{};
};
