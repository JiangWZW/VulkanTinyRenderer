#include "pch.h"
#include "TinyVulkanRenderer.h"

TinyVulkanRenderer::TinyVulkanRenderer()
= default;

TinyVulkanRenderer::~TinyVulkanRenderer()
= default;

void TinyVulkanRenderer::SetWindow(GLFWwindow* window)
{
	window_ = window;
}

int TinyVulkanRenderer::InitVkObjects()
{
	try
	{
		CreateBasicVkObjects();
	}
	catch (std::runtime_error err)
	{
		std::cout << err.what();
		return EXIT_FAILURE; // 1
	}

	return EXIT_SUCCESS; // 0
}

int TinyVulkanRenderer::Draw()
{
	try
	{
		/*system("cls");
		
		PrintBar(0, true);
		std::cout << "Drawing...\n";*/
		SubmitAndPresentVkDrawCommands();
		/*std::cout << "Succeeded to draw.\n";
		PrintBar(0, false);*/
	}
	catch (std::runtime_error err)
	{
		std::cout << err.what();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void TinyVulkanRenderer::CreateBasicVkObjects()
{
	CreateObjectWithLog(&TinyVulkanRenderer::CreateVkInstance, "VkInstance");

	CreateObjectWithLog(&TinyVulkanRenderer::CreateVkSurface, "VkSurface");

	CreateObjectWithLog(&TinyVulkanRenderer::QueryVkPhysicalDevice, "VkPhysicalDevice");

	CreateObjectWithLog(&TinyVulkanRenderer::CreateVkDevice, "VkDevice");

	CreateObjectWithLog(&TinyVulkanRenderer::QueryVkQueues, "VkQueue List");

	CreateObjectWithLog(&TinyVulkanRenderer::CreateVkSwapChain, "Swap Chain");

	CreateObjectWithLog(&TinyVulkanRenderer::CreateVkRenderPasses, "Render Pass List");

	vk_pipelines_.resize(1); // TODO: more pipelines
	CreateObjectWithLog(&TinyVulkanRenderer::CreateVkGraphicsPipeline, "Graphics Pipeline");

	CreateObjectWithLog(&TinyVulkanRenderer::CreateVkCommandPool, "Command Buffer Pool");
	
	vk_cbs_draw_.resize(1); // TODO: more command buffers
	CreateObjectWithLog(&TinyVulkanRenderer::CreateVkCommandBuffers, "Command Buffer List");

	PrintBar(0, true);
	std::cout << "Recording Draw Commands...\n";
	RecordVkDrawCommands();
	std::cout << "Succeeded to record draw commands.\n";
	PrintBar(0, false);
}

void TinyVulkanRenderer::DestroyVkObjects()
{
	vkDeviceWaitIdle(vk_device_); // wait for all device ops finished
	
	vkDestroyCommandPool(vk_device_, vk_cmd_pool_, nullptr);

	for (const auto& pipe_layout : vk_pipeline_layouts_)
	{
		vkDestroyPipelineLayout(vk_device_, pipe_layout, nullptr);
	}
	for (const auto& pipeline : vk_pipelines_)
	{
		vkDestroyPipeline(vk_device_, pipeline, nullptr);
	}

	for (const auto& render_pass : vk_render_passes_)
	{
		vkDestroyRenderPass(vk_device_, render_pass, nullptr);
	}

	swapchain_sync_objs_.Destroy(vk_device_);
	swapchain_framebuffers_.Destroy(vk_device_);
	swapchain_imgs_.Destroy(vk_device_);

	vkDestroySwapchainKHR(vk_device_, vk_swapchain_, nullptr);
	vkDestroySurfaceKHR(vk_instance_, vk_surface_, nullptr);
	vkDestroyDevice(vk_device_, nullptr);
	vkDestroyInstance(vk_instance_, nullptr);
}

void TinyVulkanRenderer::QueryVkPhysicalDevice()
{
	uint32_t num_devices = 0;
	vkEnumeratePhysicalDevices(vk_instance_, &num_devices, nullptr);
	std::vector<VkPhysicalDevice> phys_devices(num_devices);
	vkEnumeratePhysicalDevices(vk_instance_, &num_devices, phys_devices.data());
	if (num_devices == 0)
	{
		throw std::runtime_error("Cannot find any VkPhysicalDevice.");
	}


	bool fallback = true;
	for (VkPhysicalDevice device : phys_devices)
	{
		devc_queue_config_.SetQueueFamilyIndices(device, vk_surface_);
		vk_physical_device_ = device; // Plug device in

		VkPhysicalDeviceProperties dev_props = {};
		vkGetPhysicalDeviceProperties(device, &dev_props);

		if (CheckVkPhysDeviceType(dev_props.deviceType) &&
			CheckQueueFamilySupportByVkPhysDevice(this) &&
			CheckExtensionSupportByVkPhysDevice(this))
		{
			fallback = false;

			std::cout << "Pick device " << dev_props.deviceName << std::endl;

			break;
		}
	}

	if (fallback)
	{
		vk_physical_device_ = phys_devices[0];
		VkPhysicalDeviceProperties dev_props = {};
		vkGetPhysicalDeviceProperties(vk_physical_device_, &dev_props);

		std::cout << "Cannot find VkPhysicalDevice with device type 'VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU'\n";
		std::cout << "Fall back to default device: " << dev_props.deviceName << std::endl;
	}

	if (!CheckQueueFamilySupportByVkPhysDevice(this))
	{
		throw std::runtime_error("VkPhysicalDevice does not have required queue family.");
	}
	if (!CheckExtensionSupportByVkPhysDevice(this))
	{
		throw std::runtime_error("VkPhysicalDevice does not have required extension.");
	}

	devc_queue_config_.SetQueueFamilyIndices(vk_physical_device_, vk_surface_);
}

void TinyVulkanRenderer::QueryVkQueues()
{
	const QueueFamilyIndices& qfams = devc_queue_config_.GetQueueFamilyIndices();
	vkGetDeviceQueue(vk_device_, qfams.graphics_family, 0, &vk_graphics_queue_);
	vkGetDeviceQueue(vk_device_, qfams.presentation_family, 0, &vk_present_queue_);
}

void TinyVulkanRenderer::CreateVkInstance()
{
	//							VkApplicationInfo
	// ------------------------------------------------------------------------
	// Note: Always remember to use brace initializer{} for vk-related structs,
	// VkApplicationInfo applicationInfo;		--- Wrong, with random values
	// VkApplicationInfo applicationInfo = {};	--- Correct, with all 0s
	// see "https://stackoverflow.com/questions/5914422/proper-way-to-initialize-c-structs"
	VkApplicationInfo applicationInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
	applicationInfo.apiVersion = VK_API_VERSION_1_1;
	applicationInfo.pApplicationName = "Tiny Vulkan Renderer";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "Tiny Vulkan Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

	//							VkInstanceCreateInfo
	// -----------------------------------------------------------------------------------
	// Creating vk-instance-extension list
	std::vector<const char*> vk_inst_extensions = std::vector<const char*>();

	// --- Extensions required by GLFW
	uint32_t glfw_ext_count = 0;
	const char** glfw_req_extensions = glfwGetRequiredInstanceExtensions(&glfw_ext_count);
	for (uint32_t ext_id = 0; ext_id < glfw_ext_count; ++ext_id)
	{
		vk_inst_extensions.push_back(glfw_req_extensions[ext_id]);
	}

	if (!CheckExtensionSupportByVkInstance(vk_inst_extensions))
	{
		throw std::runtime_error("Instance extensions are not supported by current VkInstance.");
	}


	// Creating vk-layer list
	std::vector<const char*> vk_inst_layers = {"VK_LAYER_KHRONOS_validation"};
	if (!CheckLayerSupportByVkInstance(vk_inst_layers))
	{
		throw std::runtime_error("Validation layer not supported by current VkInstance.");
	}


	VkInstanceCreateInfo instance_create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
	instance_create_info.pApplicationInfo = &applicationInfo;

	instance_create_info.enabledExtensionCount = static_cast<uint32_t>(vk_inst_extensions.size());
	instance_create_info.ppEnabledExtensionNames = vk_inst_extensions.data();

	instance_create_info.enabledLayerCount = static_cast<uint32_t>(vk_inst_layers.size());
	instance_create_info.ppEnabledLayerNames = vk_inst_layers.data();


	// Create VkInstance
	VkResult res = vkCreateInstance(&instance_create_info, nullptr, &vk_instance_);
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create instance.");
	}
}

void TinyVulkanRenderer::CreateVkDevice()
{
	const QueueFamilyIndices& quefam_indices = devc_queue_config_.GetQueueFamilyIndices();

	float priority = 1; // For now, all queues have same priority of 1.
	std::set<int> devc_quefam_indices = {
		quefam_indices.presentation_family,
		quefam_indices.graphics_family,
		// quefam_indices.compute_family,
		// quefam_indices.transfer_family,
		// quefam_indices.sparse_binding_family
	};
	std::vector<VkDeviceQueueCreateInfo> devc_queue_create_infos(0);
	// Each VkDeviceQueueCreateInfo tells how you want to create
	// all the queues of a certain Queue Family.
	for (int quefam_idx : devc_quefam_indices)
	{
		VkDeviceQueueCreateInfo queue_create_info = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
		queue_create_info.queueFamilyIndex = quefam_idx;
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = &priority;

		devc_queue_create_infos.push_back(queue_create_info);
	}


	VkDeviceCreateInfo create_info_devc = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
	create_info_devc.queueCreateInfoCount = static_cast<uint32_t>(devc_queue_create_infos.size());
	create_info_devc.pQueueCreateInfos = devc_queue_create_infos.data();
	create_info_devc.enabledExtensionCount = static_cast<uint32_t>(kDeviceExtensions.size());
	create_info_devc.ppEnabledExtensionNames = kDeviceExtensions.data();


	const VkResult res = vkCreateDevice(vk_physical_device_, &create_info_devc, nullptr, &vk_device_);

	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create VkInstance.");
	}
}

void TinyVulkanRenderer::CreateVkSurface()
{
	const VkResult res = glfwCreateWindowSurface(vk_instance_, window_, nullptr, &vk_surface_);
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a vulkan surface");
	}
}

void TinyVulkanRenderer::CreateVkSwapChain()
{
	swapchain_config_.FetchSwapChainConfigs(vk_physical_device_, vk_surface_);

	VkSurfaceCapabilitiesKHR capabilities = swapchain_config_.GetSurfaceCapabilities();
	VkSurfaceFormatKHR format = swapchain_config_.GetSurfaceFormat();
	VkPresentModeKHR present_mode = swapchain_config_.GetPresentationMode();
	VkExtent2D extent = swapchain_config_.GetSurfaceExtent(window_);


	VkSwapchainCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
	create_info.surface = vk_surface_;
	create_info.imageFormat = format.format;
	create_info.imageColorSpace = format.colorSpace;
	create_info.imageExtent = extent;
	create_info.minImageCount = swapchain_config_.GetMinImageCount();
	create_info.imageArrayLayers = 1; // #layers for each image in chain
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Draw into color buffer
	create_info.preTransform = capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // No blending
	// Whether to clip parts of image not in view (e.g behind another window, offscreen, etc)
	create_info.clipped = VK_TRUE;

	const QueueFamilyIndices& quefam_indices = devc_queue_config_.GetQueueFamilyIndices();
	// Check if
	// Presentation Queue(present to screen)
	// & Graphics Queue(drawing to texture)
	// are in the same family;
	// If not, we need to make extra effort to make sure that
	// images in swap chain are shared by those 2 different queue families.
	if (quefam_indices.presentation_family != quefam_indices.graphics_family)
	{
		uint32_t swapchain_quefam_indices[] = {
			quefam_indices.presentation_family,
			quefam_indices.graphics_family
		};
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2; // # queues that share this resource
		create_info.pQueueFamilyIndices = swapchain_quefam_indices;
	}
	else
	{
		// Default Scenario, where Graphics Queue Family == Presentation Queue Family
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = nullptr;
	}

	// For window resizing?
	create_info.oldSwapchain = nullptr;

	vkCreateSwapchainKHR(vk_device_, &create_info, nullptr, &vk_swapchain_);

	// Create swapchain images & image-views
	swapchain_imgs_.Create(vk_device_, vk_swapchain_, format.format);
	// Create swapchain synchronization primitives
	swapchain_sync_objs_.Create(vk_device_);
}

void TinyVulkanRenderer::CreateVkRenderPasses()
{
	// === Attachment Create Infos ===
	// -------------------------------
	// -- Attachment Descriptions --
/**
 * \brief How many attachments are inside main render pass
 */
#define ATTACHMENT_COUNT_MAIN_PASS 1
	std::array<VkAttachmentDescription, ATTACHMENT_COUNT_MAIN_PASS> attachment_descs{};
	VkAttachmentDescription attach_desc = {};
	// ------------------------------------------------------------------
	// Note: Coherent attachment format between framebuffer && renderpass
	// If flags does not include VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT,
	// VkFormat value of each attachment
	// in frame buffer				--- defined in VkImageView
	// must match with renderPass	--- defined in VkAttachmentDescription(here)
	attach_desc.format = swapchain_config_.GetSurfaceFormat().format;
	// ------------------------------------------------------------------
	attach_desc.samples        = VK_SAMPLE_COUNT_1_BIT;
	attach_desc.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear before load
	attach_desc.storeOp        = VK_ATTACHMENT_STORE_OP_STORE; // Just store
	attach_desc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // Stencil not in use for now
	attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // ---------------------------------
	attach_desc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED; // Expected image layout before pass
	attach_desc.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Output image layout after pass

	attachment_descs[0] = attach_desc;

	// -- Attachment References --
	// Subpass holds internal attachment handles as reference
#define ATTACHMENT_COUNT_SUBPASS_0 ATTACHMENT_COUNT_MAIN_PASS
	std::array<VkAttachmentReference, ATTACHMENT_COUNT_SUBPASS_0> attachment_refs{};
	VkAttachmentReference color_attach_ref = {};
	color_attach_ref.attachment = 0; // 0th attachment binding
	color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachment_refs[0] = color_attach_ref;


	// === Subpass Create Infos ===
	// -------------------------------
	// -- Subpass Descriptions --
#define SUBPASS_COUNT_MAIN_PASS 1
	std::array<VkSubpassDescription, SUBPASS_COUNT_MAIN_PASS> subpass_descs{};
	VkSubpassDescription subpass_desc = {};
	subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_desc.colorAttachmentCount = 1;
	subpass_desc.pColorAttachments = attachment_refs.data();

	subpass_descs[0] = subpass_desc;

	// -- Subpass Dependencies --
	std::array<VkSubpassDependency, SUBPASS_COUNT_MAIN_PASS> subpass_dependencies{};
	VkSubpassDependency subpass_dep = {};
	subpass_dep.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpass_dep.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpass_dep.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;

	subpass_dep.dstSubpass = 0;
	subpass_dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dep.dstAccessMask =
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass_dep.dependencyFlags = 0; // no dependency flags

	subpass_dependencies[0] = subpass_dep;


	VkRenderPassCreateInfo render_pass_ci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	// Image attachments ------------------------------------
	render_pass_ci.attachmentCount = attachment_descs.size();
	render_pass_ci.pAttachments = attachment_descs.data();
	// Sub-passes dependency graph -----------------------------
	render_pass_ci.subpassCount = subpass_descs.size();
	render_pass_ci.pSubpasses = subpass_descs.data();
	render_pass_ci.dependencyCount = subpass_dependencies.size();
	render_pass_ci.pDependencies = subpass_dependencies.data();

	VkRenderPass render_pass_main;
	VkResult res = vkCreateRenderPass(vk_device_, &render_pass_ci, nullptr, &render_pass_main);
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create main render pass");
	}
	vk_render_passes_.push_back(render_pass_main);

	// -- Frame Buffers for swapchain images --
	const int swapchain_img_count = swapchain_imgs_.ImageCount();
	std::vector<VkExtent2D> swapchain_framebuffer_extents(swapchain_img_count);

	for (int i = 0; i < swapchain_img_count; ++i)
	{
		swapchain_framebuffer_extents[i] = swapchain_config_.GetSurfaceExtent(window_);
	}

	swapchain_framebuffers_.Create(
		vk_device_,
		render_pass_main,
		swapchain_img_count,
		swapchain_imgs_.GetImageViews().data(),
		swapchain_framebuffer_extents.data()
	);


	// ... More render passes can be created here...
}

void TinyVulkanRenderer::CreateVkGraphicsPipeline()
{
	// TODO: Automate shader compile path on vs-code side
	const VkShaderModule& vertex_shader =
		ShaderLoader::LoadShaderModule(vk_device_, "Shaders/vert.spv");
	const VkShaderModule& fragment_shader =
		ShaderLoader::LoadShaderModule(vk_device_, "Shaders/frag.spv");

	VkGraphicsPipelineCreateInfo pipeline_ci = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

	// Shader Stages
	// ---------------------------------------------------------------------------------------
	std::array<VkPipelineShaderStageCreateInfo, 2> shader_stage_create_infos{};

	VkPipelineShaderStageCreateInfo vs_ci = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
	vs_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vs_ci.module = vertex_shader;
	vs_ci.pName = "main"; // Entry point in to shader
	shader_stage_create_infos[0] = vs_ci;

	VkPipelineShaderStageCreateInfo fs_ci = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
	fs_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fs_ci.module = fragment_shader;
	fs_ci.pName = "main";
	shader_stage_create_infos[1] = fs_ci;

	// Fixed Function Stages
	// ------------------------------------------------------------------------------------
	// --- Vertex Input Layout ---
	VkPipelineVertexInputStateCreateInfo vertex_input_ci = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	vertex_input_ci.vertexBindingDescriptionCount = 0; // TODO: setup vertex input
	vertex_input_ci.pVertexBindingDescriptions = nullptr;
	vertex_input_ci.vertexAttributeDescriptionCount = 0;
	vertex_input_ci.pVertexAttributeDescriptions = nullptr;

	// --- Mesh Assembly ---
	VkPipelineInputAssemblyStateCreateInfo input_assembly_ci = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
	};
	input_assembly_ci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	// --- primitiveRestartEnable ---
	// Allow mesh assembler to restart new (line/triangle)strips.
	// 
	// If primitiveRestartEnable is VK_TRUE,
	// then a special “index” indicates that the primitive should start over.
	// --- 2 Vertex Index Formats ---
	// VK_INDEX_TYPE_UINT16 = 0, // 0 – 65,535
	// VK_INDEX_TYPE_UINT32 = 1, // 0 – 4,294,967,295
	// If your VkIndexType is VK_INDEX_TYPE_UINT16, then the special index is 0xffff.
	// If your VkIndexType is VK_INDEX_TYPE_UINT32, it is 0xffffffff.
	//
	// --- Why use this ---
	// This is more efficient than explicitly ending the current primitive and
	// explicitly starting a new primitive of the same type.
	input_assembly_ci.primitiveRestartEnable = VK_FALSE;

	// --- Viewport Transform ---
	VkPipelineViewportStateCreateInfo viewport_state_ci = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
	const VkExtent2D surface_size = swapchain_config_.GetSurfaceExtent(window_);

	viewport_state_ci.viewportCount = 1;
	VkViewport viewport_main = {
		0, 0,
		static_cast<float>(surface_size.width),
		static_cast<float>(surface_size.height),
		0, 1
	};
	viewport_state_ci.pViewports = &viewport_main;

	viewport_state_ci.scissorCount = 1;
	VkRect2D scissor = {
		{0, 0},
		surface_size
	};
	viewport_state_ci.pScissors = &scissor;

	// --- Rasterization States ---
	VkPipelineRasterizationStateCreateInfo raster_state_ci = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
	};
	raster_state_ci.depthClampEnable = VK_FALSE;
	raster_state_ci.rasterizerDiscardEnable = VK_FALSE;
	raster_state_ci.polygonMode = VK_POLYGON_MODE_FILL;
	raster_state_ci.cullMode = VK_CULL_MODE_BACK_BIT;
	raster_state_ci.frontFace = VK_FRONT_FACE_CLOCKWISE;
	raster_state_ci.depthBiasEnable = VK_FALSE;
	// Note: Configure when do shadow mapping
	// raster_state_ci.depthBiasConstantFactor;
	// raster_state_ci.depthBiasClamp;
	// raster_state_ci.depthBiasSlopeFactor;
	raster_state_ci.lineWidth = 1;

	// --- Multi-Sample States ---
	VkPipelineMultisampleStateCreateInfo msaa_state_ci = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
	msaa_state_ci.sampleShadingEnable = VK_FALSE;
	msaa_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	// Note: Configure when do MSAA
	// float                 minSampleShading;
	// const                 VkSampleMask* pSampleMask;
	// VkBool32              alphaToCoverageEnable;
	// VkBool32              alphaToOneEnable;

	// --- Blending States ---
	VkPipelineColorBlendStateCreateInfo col_blend_ci = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
	col_blend_ci.logicOpEnable = VK_FALSE; // Logic ops only for integer frame-buffers
	VkPipelineColorBlendAttachmentState blend_attach_0 =
	{
		// Blending Formula:
		// srcColorBlendFactor * newColor.rgb + destColorBlendFactor * oldColor.rgb
		// srcAlphaBlendFactor * newColor.a   + destAlphaBlendFactor * oldColor.a
		VK_TRUE,
		// Color Blending
		VK_BLEND_FACTOR_SRC_ALPHA,
		VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		VK_BLEND_OP_ADD,
		// Alpha Blending
		VK_BLEND_FACTOR_ONE,
		VK_BLEND_FACTOR_ZERO,
		VK_BLEND_OP_ADD,
		// Color Channel Mask
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};
	col_blend_ci.attachmentCount = 1; // >1 when using independent blending
	col_blend_ci.pAttachments = &blend_attach_0;

	// Not Depth/Stencil for now
	// VkPipelineDepthStencilStateCreateInfo
	// const VkPipelineColorBlendStateCreateInfo* pColorBlendState;
	// const VkPipelineDynamicStateCreateInfo* pDynamicState;


	// Pipeline Layout
	// -- PIPELINE LAYOUT (TODO: Apply Future Descriptor Set Layouts) --
	vk_pipeline_layouts_.clear();

	VkPipelineLayoutCreateInfo pipe_layout_ci = {};
	pipe_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipe_layout_ci.setLayoutCount = 0;
	pipe_layout_ci.pSetLayouts = nullptr;
	pipe_layout_ci.pushConstantRangeCount = 0;
	pipe_layout_ci.pPushConstantRanges = nullptr;

	// Create Pipeline Layout
	VkPipelineLayout pipe_layout;
	VkResult result = vkCreatePipelineLayout(vk_device_, &pipe_layout_ci, nullptr, &pipe_layout);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Pipeline Layout!");
	}
	vk_pipeline_layouts_.push_back(pipe_layout);


	// Fill data_ prepared above into pipeline create info
	// --------------------------------------------------
	// Shader Stages
	pipeline_ci.stageCount = 2;
	pipeline_ci.pStages = shader_stage_create_infos.data();
	// Fixed Function States
	pipeline_ci.pVertexInputState = &vertex_input_ci;
	pipeline_ci.pInputAssemblyState = &input_assembly_ci;
	pipeline_ci.pViewportState = &viewport_state_ci;
	pipeline_ci.pRasterizationState = &raster_state_ci;
	pipeline_ci.pMultisampleState = &msaa_state_ci;
	pipeline_ci.pDepthStencilState = nullptr; // TO be added
	pipeline_ci.pColorBlendState = &col_blend_ci;
	pipeline_ci.pDynamicState = nullptr;
	pipeline_ci.layout = vk_pipeline_layouts_[0];
	pipeline_ci.renderPass = vk_render_passes_[0];
	pipeline_ci.subpass = 0; // subpass of render pass to use with pipeline
	// Each pipeline only uses 1 subpass,
	// for multi-sub-passes, try duplicate more pipelines

	// -- Pipeline derivative -- 
	// Can create multiple pipelines derive from another for optimization
	// For details, see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#pipelines-pipeline-derivatives
	pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_ci.basePipelineIndex = -1;

	VkResult res = vkCreateGraphicsPipelines(
		vk_device_,
		VK_NULL_HANDLE,
		1,
		&pipeline_ci,
		nullptr,
		vk_pipelines_.data());
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline");
	}

	vkDestroyShaderModule(vk_device_, vertex_shader, nullptr);
	vkDestroyShaderModule(vk_device_, fragment_shader, nullptr);
}

void TinyVulkanRenderer::CreateVkCommandPool()
{
	VkCommandPoolCreateInfo cmdpool_ci = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	cmdpool_ci.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	cmdpool_ci.queueFamilyIndex = devc_queue_config_.GetQueueFamilyIndices().graphics_family;

	VK_CHECK(vkCreateCommandPool(vk_device_, &cmdpool_ci, nullptr, &vk_cmd_pool_),
	         "Failed to create a command buffer pool.");
}

void TinyVulkanRenderer::CreateVkCommandBuffers()
{
	// Primary command buffers(for now)
	// --- 1 prim cb per swapchain image
	vk_cbs_draw_.resize(swapchain_imgs_.GetImages().size());

	VkCommandBufferAllocateInfo cmd_alloc_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	cmd_alloc_info.commandPool = vk_cmd_pool_;
	// Parent cb for executing secondary cbs : VK_COMMAND_BUFFER_LEVEL_PRIMARY
	cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd_alloc_info.commandBufferCount = vk_cbs_draw_.size();
	VK_CHECK( // Create cb here
		vkAllocateCommandBuffers(vk_device_, &cmd_alloc_info, vk_cbs_draw_.data()),
		"Failed to create primary command buffer"
	);
}

void TinyVulkanRenderer::RecordVkDrawCommands()
{
	VkCommandBufferBeginInfo cb_beg_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	// cb can be re-submitted without reset/alloc
	// cb_beg_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;


	const VkExtent2D surface_extent = swapchain_config_.GetSurfaceExtent(window_);
	const VkClearColorValue clear_color_value =
		{{1.0, 0.8, 0.4, 1.0}};
	VkClearValue clear_values[1] = {
		{clear_color_value}
	};

	VkRenderPassBeginInfo draw_pass_beg_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
	draw_pass_beg_info.renderPass = vk_render_passes_[0];
	// Note: A render pass with renderArea must within the bound of the framebuffer
	draw_pass_beg_info.renderArea =
		VkRect2D{
			0, 0,
			{
				surface_extent.width,
				surface_extent.height
			}
		};
	// Note: clearValueCount must be greater than
	// the largest attachment index in renderPass that specifies
	// a loadOp of VK_ATTACHMENT_LOAD_OP_CLEAR.
	// (or stencilLoadOp, if the attachment has a depth/stencil format)
	draw_pass_beg_info.clearValueCount = ATTACHMENT_COUNT_MAIN_PASS;
	draw_pass_beg_info.pClearValues = clear_values;

	for (size_t swapchain_img_id = 0; swapchain_img_id < swapchain_imgs_.ImageCount(); swapchain_img_id++)
	{
		VkCommandBuffer current_cb = vk_cbs_draw_[swapchain_img_id];
		// Update target framebuffer in render pass
		draw_pass_beg_info.framebuffer = swapchain_framebuffers_[swapchain_img_id];

		// Start recording commands
		vkBeginCommandBuffer(current_cb, &cb_beg_info);
		{
			vkCmdBeginRenderPass(current_cb, &draw_pass_beg_info, {VK_SUBPASS_CONTENTS_INLINE});
			{
				vkCmdBindPipeline(current_cb, {VK_PIPELINE_BIND_POINT_GRAPHICS}, vk_pipelines_[0]);
				
				vkCmdDraw(current_cb, 3, 1, 0, 0);
			}
			vkCmdEndRenderPass(current_cb);
		}
		VK_CHECK(vkEndCommandBuffer(current_cb), "Failed to record command buffer");
	}
}

#define MAX_WAIT_TIME (std::numeric_limits<uint32_t>::max())

void TinyVulkanRenderer::SubmitAndPresentVkDrawCommands()
{
	// Setup Semaphores & fences
	const VkSemaphore& semaphore_acquire = swapchain_sync_objs_.AcquireSemaphore(frame_counter_);
	const VkSemaphore& semaphore_present = swapchain_sync_objs_.PresentSemaphore(frame_counter_);
	// This fence will be signaled by VkQueueSubmit
	// once all submitted command buffers have completed execution,
	// In order to stop CPU from constantly submitting unnecessary cbs to GPU
	const VkFence& fence_cb_done = swapchain_sync_objs_.CmdBufferFence(frame_counter_);

	vkWaitForFences(vk_device_, 1, &fence_cb_done, true, MAX_WAIT_TIME);
	vkResetFences(vk_device_, 1, &fence_cb_done);

	
	// All command submission & presentation ops only after previous fence is signaled
	{
		// Submit draw commands recorded for current swap chain image to graphics queue
		// ------------------------------------------------------------------------------
		uint32_t img_id = 0;

		VK_CHECK(vkAcquireNextImageKHR(vk_device_, vk_swapchain_,
			MAX_WAIT_TIME,
			semaphore_acquire,
			nullptr,
			&img_id
		), "Failed to acquire next image from swapchain");

		VkSubmitInfo submit_info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &(semaphore_acquire);
		submit_info.pWaitDstStageMask = &wait_stage;

		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &(vk_cbs_draw_[img_id]);
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &(semaphore_present);

		VK_CHECK( // Submit command buffer to queue
			vkQueueSubmit(
				vk_graphics_queue_, 1, &submit_info,
				fence_cb_done // a fence to be signaled once all 
				// submitted command buffers have completed execution
			),
			"Failed to submit draw commands to graphics queue"
		);


		// Present swapchain image to presentation queue for displaying on screen
		// -----------------------------------------------------------------------
		VkPresentInfoKHR present_info = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &(semaphore_present);
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &vk_swapchain_;
		present_info.pImageIndices = &img_id;

		VK_CHECK(vkQueuePresentKHR(vk_present_queue_, &present_info),
			"Failed to present swap chain image to presentation queue");


		// Lastly, Update frame counter
		frame_counter_ = (frame_counter_ + 1) % 2;
	}
}


bool TinyVulkanRenderer::CheckLayerSupportByVkInstance(const std::vector<const char*>& layers)
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> available_layers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, available_layers.data());

	for (const char* layer_name : layers)
	{
		bool layer_found = false;

		for (const auto& layer_properties : available_layers)
		{
			if (strcmp(layer_name, layer_properties.layerName) == 0)
			{
				layer_found = true;
				break;
			}
		}

		if (!layer_found)
		{
			return false;
		}
	}

	return true;
}

bool TinyVulkanRenderer::CheckExtensionSupportByVkInstance(std::vector<const char*> extensions)
{
	PrintBar(1, true);
	std::cout << "Checking Support for Instance Extensions...." << std::endl;

	// Fetch supported extensions by current vkInstance by enumerating
	// -------------------------------------------------------------------
	uint32_t vk_inst_ext_count = 0;
	vkEnumerateInstanceExtensionProperties(
		nullptr,
		&vk_inst_ext_count,
		nullptr
	);
	std::vector<VkExtensionProperties> supported_vk_exts(vk_inst_ext_count);
	vkEnumerateInstanceExtensionProperties(
		nullptr,
		&vk_inst_ext_count,
		supported_vk_exts.data()
	);


	// Check if all instance extensions are supported,
	// simply by finding them in supported ones
	// -------------------------------------------------------------------
	std::cout << std::setw(20) << std::left << "Extension Name";
	std::cout << std::setw(20) << std::right << "Supported" << std::endl;

	for (const auto* ext_needed : extensions)
	{
		std::cout << std::left << std::setw(20) << ext_needed;
		bool is_ext_need_supported = false;
		for (const auto& ext_supported : supported_vk_exts)
		{
			if (strcmp(ext_supported.extensionName, ext_needed) == 0)
			{
				is_ext_need_supported = true;
				break;
			}
		}
		std::cout << std::right << std::setw(20) <<
			(is_ext_need_supported ? "Yes\n" : "No, Exit Process...\n");
		if (!is_ext_need_supported)
		{
			return false;
		}
	}
	std::cout << "\nAll Supported.\n";
	PrintBar(1, false);

	return true;
}

bool TinyVulkanRenderer::CheckExtensionSupportByVkPhysDevice(const TinyVulkanRenderer* renderer)
{
	uint32_t num_devc_props = 0;
	vkEnumerateDeviceExtensionProperties(
		renderer->vk_physical_device_, nullptr, &num_devc_props, nullptr);

	std::vector<VkExtensionProperties> devc_ext_props(num_devc_props);
	vkEnumerateDeviceExtensionProperties(
		renderer->vk_physical_device_, nullptr, &num_devc_props, devc_ext_props.data());


	for (const char* ext_name : kDeviceExtensions)
	{
		bool ext_supported = false;
		for (const auto& ext_prop : devc_ext_props)
		{
			if (strcmp(ext_prop.extensionName, ext_name) == 0)
			{
				ext_supported = true;
				break;
			}
		}

		if (!ext_supported)
		{
			return false;
		}
	}

	return true;
}

bool TinyVulkanRenderer::CheckVkPhysDeviceType(const VkPhysicalDeviceType type)
{
	return (type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
}

bool TinyVulkanRenderer::CheckQueueFamilySupportByVkPhysDevice(const TinyVulkanRenderer* renderer)
{
	return renderer->devc_queue_config_.GetQueueFamilyIndices().AllQueueFamiliesValid();
}


bool TinyVulkanRenderer::CheckPresentationQueueSupport(
	const VkPhysicalDevice device,
	const VkSurfaceKHR surface,
	const uint32_t quefam_idx)
{
	VkBool32 supported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(
		device, quefam_idx, surface, &supported);

	return static_cast<bool>(supported);
}

void TinyVulkanRenderer::PrintBar(const int level, const bool start)
{
	if (start)
	{
		std::cout << std::endl;
	}
	for (int i = 0; i < 3 - level; ++i)
	{
		std::cout << "--------------";
	}
	for (int i = 0; i < level; ++i)
	{
		std::cout << "- - - - - - - ";
	}
	if (!start)
	{
		std::cout << std::endl;
	}
	std::cout << std::endl;
}


void TinyVulkanRenderer::CreateObjectWithLog(void (TinyVulkanRenderer::* func)(), const std::string& object_name)
{
	PrintBar(0, true);
	std::cout << "Creating " + object_name + "...\n";

	(this->*func)(); // Actual function to create vk-related object

	std::cout << "Succeeded to create " + object_name + ".\n";
	PrintBar(0, false);
}
