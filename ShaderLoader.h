#pragma once

#include "pch.h"

class ShaderLoader
{
public:
	static VkShaderModule LoadShaderModule(VkDevice device, const std::string& shader_path);

	
private:
	static void LoadShaderFile(std::vector<char>& out_buffer, const std::string& shader_path);
};
