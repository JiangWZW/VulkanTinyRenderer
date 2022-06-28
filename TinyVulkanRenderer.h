#pragma once

#include "pch.h"

#include "DevcQueueConfigurator.h"
#include "SwapChainConfigurator.h"
#include "SwapChainImages.h"
#include "SwapChainFramebuffers.h"
#include "ShaderLoader.h"
#include "SwapChainSyncPrimitives.h"


const std::vector<const char*> kDeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

class TinyVulkanRenderer
{
public:
	TinyVulkanRenderer();
	~TinyVulkanRenderer();

	void SetWindow(GLFWwindow* window);

	// Init VK Objects
	int InitVkObjects();
	int Draw();
	
	// Release VK Objects
	void DestroyVkObjects();

private:
	GLFWwindow*                   window_             = nullptr;

	// Vulkan Objects
	VkInstance                    vk_instance_;
	VkPhysicalDevice              vk_physical_device_;
	VkDevice                      vk_device_;
	VkQueue                       vk_graphics_queue_;
	VkQueue                       vk_present_queue_;
	VkSurfaceKHR                  vk_surface_;
	VkSwapchainKHR                vk_swapchain_;

	std::vector<VkRenderPass>     vk_render_passes_;
	std::vector<VkPipeline>       vk_pipelines_;
	std::vector<VkPipelineLayout> vk_pipeline_layouts_;
	std::vector<VkCommandBuffer>  vk_cbs_draw_;

	VkCommandPool                 vk_cmd_pool_;

	SwapChainImages               swapchain_imgs_;
	SwapChainFramebuffers         swapchain_framebuffers_;
	SwapChainSyncPrimitives		  swapchain_sync_objs_;
	
	
	// Get VK Objects
	void QueryVkPhysicalDevice();
	void QueryVkQueues();


	// Create VK Objects
	void CreateBasicVkObjects();

	void CreateVkInstance();
	void CreateVkDevice();

	void CreateVkSurface();
	void CreateVkSwapChain();

	void CreateVkGraphicsPipeline();

	void CreateVkRenderPasses();

	void CreateVkCommandPool();
	void CreateVkCommandBuffers();

	void RecordVkDrawCommands();
	void SubmitAndPresentVkDrawCommands();
	
	// Support Objects
	SwapChainConfigurator swapchain_config_;
	DevcQueueConfigurator devc_queue_config_;
	int frame_counter_ = 0;

	// Support Functions
	static bool CheckLayerSupportByVkInstance(const std::vector<const char*> & layers);
	static bool CheckExtensionSupportByVkInstance(std::vector<const char*> extensions);

	static bool CheckExtensionSupportByVkPhysDevice(const TinyVulkanRenderer* renderer);
	static bool CheckQueueFamilySupportByVkPhysDevice(const TinyVulkanRenderer* renderer);
	static bool CheckVkPhysDeviceType(const VkPhysicalDeviceType type);

	static bool CheckPresentationQueueSupport(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t quefam_idx);


	// Utility Functions
	static void PrintBar(const int level = 0, const bool start = true);
	void CreateObjectWithLog(void (TinyVulkanRenderer::*func)(), const std::string& object_name);
};
