#pragma once

struct RenderPipelineCreateInfo
{
	VkDevice device;
	std::string shader_path_vert;
	std::string shader_path_frag;
	
};


class RenderPipeline
{
public:
private:
	RenderPipeline() { vk_pipeline_ = nullptr; }
	VkPipeline vk_pipeline_;
};

