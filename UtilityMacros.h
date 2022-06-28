#pragma once

constexpr void VK_CHECK(VkResult res, const std::string& errstr)
{
	if (res != VK_SUCCESS) 
	{
			throw std::runtime_error(errstr);
	}
}