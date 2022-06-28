#include "pch.h"
#include "DevcQueueConfigurator.h"

#include <vector>

void DevcQueueConfigurator::SetQueueFamilyIndices(
	const VkPhysicalDevice& device,
	const VkSurfaceKHR& surface
)
{
	uint32_t quefam_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(
		device, &quefam_count, nullptr);
	std::vector<VkQueueFamilyProperties> quefam_props_list(quefam_count);
	vkGetPhysicalDeviceQueueFamilyProperties(
		device, &quefam_count, quefam_props_list.data());

	int id = 0;
	for (const auto& props : quefam_props_list)
	{
		if (props.queueCount > 0)
		{
			if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				queue_family_ids_.graphics_family = id;
			}
			if (props.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				queue_family_ids_.compute_family = id;
			}
			if (props.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				queue_family_ids_.transfer_family = id;
			}
			if (props.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
			{
				queue_family_ids_.sparse_binding_family = id;
			}
			if (CheckPresentationQueueSupport(device, surface, id))
			{
				queue_family_ids_.presentation_family = id;
			}
		}

		++id;
	}
}

const QueueFamilyIndices& DevcQueueConfigurator::GetQueueFamilyIndices() const
{
	return queue_family_ids_;
}

bool DevcQueueConfigurator::CheckPresentationQueueSupport(
	const VkPhysicalDevice& device,
	const VkSurfaceKHR& surface,
	const uint32_t queue_family_id)
{
	VkBool32 supported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(
		device, queue_family_id, surface, &supported);

	return static_cast<bool>(supported);
}
