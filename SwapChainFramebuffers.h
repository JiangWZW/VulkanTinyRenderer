#pragma once

class SwapChainFramebuffers
{
public:
	void Create(
		VkDevice device, VkRenderPass renderpass,
		const int img_count,
		const VkImageView* const img_views, const VkExtent2D* const img_extents);

	void Destroy(VkDevice device);

	// Overload
	VkFramebuffer operator[](const int i)
	{
		if (0 <= i && i < data_.size())
		{
			return data_[i];
		}
		return nullptr;
	}

private:
	std::vector<VkFramebuffer> data_;
};
