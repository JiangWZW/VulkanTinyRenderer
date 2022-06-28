#pragma once

#include "pch.h"

struct QueueFamilyIndices
{
	int graphics_family       = -1;
	int compute_family        = -1;
	int transfer_family       = -1;
	int sparse_binding_family = -1;

	int presentation_family   = -1;

	// If all queue families are supported on this device
	bool AllQueueFamiliesValid() const
	{
		return (graphics_family >= 0) &&
			(compute_family >= 0) &&
			(transfer_family >= 0) &&
			(sparse_binding_family >= 0);
	}
};

class DevcQueueConfigurator
{
public:
	void SetQueueFamilyIndices(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
	const QueueFamilyIndices& GetQueueFamilyIndices() const;
	
private:
	QueueFamilyIndices queue_family_ids_;

	static bool CheckPresentationQueueSupport(
		const VkPhysicalDevice& device,
		const VkSurfaceKHR& surface,
		const uint32_t queue_family_id
	);

};
