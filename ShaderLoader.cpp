#include "pch.h"
#include "ShaderLoader.h"

VkShaderModule ShaderLoader::LoadShaderModule(VkDevice device,const std::string& shader_path)
{
	std::vector<char> spirv_data(0);
	LoadShaderFile(spirv_data, shader_path);

	VkShaderModuleCreateInfo module_create_info = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	module_create_info.codeSize = spirv_data.size();
	module_create_info.pCode    = reinterpret_cast<const uint32_t*>(spirv_data.data());

	VkShaderModule shader_module;
	VkResult res = vkCreateShaderModule(device, &module_create_info, nullptr, &shader_module);
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module at: " + shader_path);
	}

	return shader_module;
}

void ShaderLoader::LoadShaderFile(
	std::vector<char>& out_buffer,
	const std::string& shader_path)
{
	std::ifstream file(shader_path, std::ios::binary | std::ios::ate);
	// start at end to get file size
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open shader file at: " + shader_path);
	}

	const int size = file.tellg(); // Tells current cursor pos at ("get"==input) input file
	file.seekg(0); // Put cursor ar start(0) for read in

	out_buffer.resize(size);
	file.read(out_buffer.data(), size);

	file.close();
}
