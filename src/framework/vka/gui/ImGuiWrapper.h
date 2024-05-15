#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "../global_state.h"
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include "../combined_resources/CmdBuffer.h"
#include "../combined_resources/Image.h"	
#include "../resources/RasterizationPipeline.h"

namespace vka
{

class ImGuiWrapper
{
  public:
	ImGuiWrapper();
	~ImGuiWrapper();

	void renderGui(UniversalCmdBuffer& cmdBuf);
	void uploadResources(CmdBuffer& cmdBuf);
	void destroyStagingResources();
	void newFrame();
	void init();
	void destroy();
  private:
	void updatFramebuffers();
	void createRenderPass();
	void createFramebuffers();
	void initImGui(VkRenderPass renderPass);
	void addGVars(GVar_Cat category);
	void addGVar(GVar *gv);
	void buildGui();
	void draw(UniversalCmdBuffer &cmdBuf);

	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> framebuffers;
	VkExtent2D framebufferExtent;
};


ImGuiWrapper::ImGuiWrapper()
{
}
ImGuiWrapper::~ImGuiWrapper()
{
}

void ImGuiWrapper::initImGui(VkRenderPass renderPass)
{
	// Init stuff
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void) io;

	float sizePixels = 18;
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/Roboto-Medium.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/Cousine-Regular.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/DroidSans.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/Karla-Regular.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/ProggyClean.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/ProggyTiny.ttf", sizePixels);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsClassic();
	gState.io.getWindow()->initGui();

	// Create Descriptor Pool
	VkDescriptorPoolSize gui_pool_sizes[] =
	    {
	        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
	        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
	        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
	        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
	        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
	        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
	        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
	        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
	        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
	        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
	        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

	VkDescriptorPoolCreateInfo guiPoolInfo = {};
	guiPoolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	guiPoolInfo.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	guiPoolInfo.maxSets                    = 1000 * IM_ARRAYSIZE(gui_pool_sizes);
	guiPoolInfo.poolSizeCount              = (uint32_t) IM_ARRAYSIZE(gui_pool_sizes);
	guiPoolInfo.pPoolSizes                 = gui_pool_sizes;

	VkResult result = vkCreateDescriptorPool(gState.device.logical, &guiPoolInfo, nullptr, &descriptorPool);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a gui descriptor pool!");
	}

	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance                  = gState.device.instance;
	initInfo.PhysicalDevice            = gState.device.physical;
	initInfo.Device                    = gState.device.logical;
	initInfo.QueueFamily               = gState.device.universalQueueFamily;
	initInfo.Queue                     = gState.device.universalQueues[0];
	initInfo.DescriptorPool            = descriptorPool;
	initInfo.PipelineCache             = VK_NULL_HANDLE;        // we do not need those
	initInfo.MinImageCount             = 2;
	initInfo.ImageCount                = gState.io.imageCount;
	initInfo.Allocator                 = VK_NULL_HANDLE;
	initInfo.CheckVkResultFn           = VK_NULL_HANDLE;
	initInfo.MSAASamples               = VK_SAMPLE_COUNT_1_BIT;
	ImGui_ImplVulkan_Init(&initInfo, renderPass);
}

void ImGuiWrapper::uploadResources(CmdBuffer& cmdBuf)
{
	ImGui_ImplVulkan_CreateFontsTexture(vka_compatibility::getHandle(cmdBuf));
}

void ImGuiWrapper::destroyStagingResources()
{
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char *desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void ImGuiWrapper::addGVar(GVar *gv)
{
	std::stringstream ss;
	switch (gv->type)
	{
		case GVAR_EVENT:
			gv->val.v_bool = ImGui::Button(gv->name.c_str());
			break;
		case GVAR_BOOL:
			ImGui::Checkbox(gv->name.c_str(), &gv->val.v_bool);
			break;
		case GVAR_FLOAT:
			ImGui::InputScalar(gv->name.c_str(), ImGuiDataType_Float, &gv->val.v_float);
			break;
		case GVAR_UNORM:
			ImGui::SliderFloat(gv->name.c_str(), &gv->val.v_float, 0.0f, 1.0f); 
			break;
		case GVAR_UINT:
			ImGui::InputScalar(gv->name.c_str(), ImGuiDataType_U32, &gv->val.v_uint);
			break;
		case GVAR_INT:
			ImGui::InputInt(gv->name.c_str(), &gv->val.v_int);
			break;
		case GVAR_VEC3:
			ImGui::InputFloat3(gv->name.c_str(), gv->val.v_vec3);
			break;
		case GVAR_DISPLAY_VALUE:
			ImGui::Text(gv->name.c_str(), gv->val.v_float);
			break;
		case GVAR_ENUM:
			for (size_t i = 0; i < gv->enumVal.size(); i++)
			{
				ss << gv->enumVal[i] << '\0';
			}
			ImGui::Combo(gv->name.c_str(), &gv->val.v_int, ss.str().c_str(), 5);
			break;
		default:
			break;
	}
}

void ImGuiWrapper::addGVars(GVar_Cat category)
{
	for (uint32_t i = 0; i < gVars.size(); i++)
	{
		if (gVars[i]->cat == category)
		{
			addGVar(gVars[i]);
		}
	}
}

void ImGuiWrapper::buildGui()
{
	ImGui::Begin("Gui");

	if (ImGui::CollapsingHeader("Application"))
	{
		addGVars(GVAR_APPLICATION);
	}

	if (ImGui::CollapsingHeader("Framework"))
	{
		addGVars(GVAR_FRAMEWORK);
	}

	ImGui::End();
}

void ImGuiWrapper::newFrame()
{
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImGuiWrapper::draw(UniversalCmdBuffer &cmdBuf)
{
	// ImGui::ShowDemoWindow();
	buildGui();
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vka_compatibility::getHandle(cmdBuf));
}

void ImGuiWrapper::destroy()
{
	// clean up of GUI stuff
	for (size_t i = 0; i < gState.io.imageCount; i++)
	{
		vkDestroyFramebuffer(gState.device.logical, framebuffers[i], nullptr);
	}
	framebuffers.clear();
	vkDestroyRenderPass(gState.device.logical, renderPass, nullptr);
	vkDestroyDescriptorPool(gState.device.logical, descriptorPool, nullptr);
	renderPass = VK_NULL_HANDLE;
	ImGui_ImplVulkan_Shutdown();
}

void ImGuiWrapper::createRenderPass()
{
	ASSERT_TRUE(renderPass == VK_NULL_HANDLE);
	VkAttachmentDescription attachmentDescription{};
	attachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachmentDescription.format         = gState.io.format;
	attachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	std::vector<VkSubpassDependency> subpassDependencies;
	{
		VkSubpassDependency subpassDependency{};
		subpassDependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
		subpassDependency.dstSubpass      = 0;
		subpassDependency.srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpassDependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency.srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
		subpassDependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		subpassDependencies.push_back(subpassDependency);
	}
	{
		VkSubpassDependency subpassDependency{};
		subpassDependency.srcSubpass      = 0;
		subpassDependency.dstSubpass      = VK_SUBPASS_EXTERNAL;
		subpassDependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency.dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpassDependency.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependency.dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
		subpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		subpassDependencies.push_back(subpassDependency);
	}

	VkAttachmentReference attachmetReference{};
	attachmetReference.attachment = 0;
	attachmetReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments    = &attachmetReference;

	VkRenderPassCreateInfo renderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments    = &attachmentDescription;
	renderPassCreateInfo.subpassCount    = 1;
	renderPassCreateInfo.pSubpasses      = &subpassDescription;
	renderPassCreateInfo.dependencyCount = subpassDependencies.size();
	renderPassCreateInfo.pDependencies   = subpassDependencies.data();

	ASSERT_VULKAN(vkCreateRenderPass(gState.device.logical, &renderPassCreateInfo, nullptr, &renderPass));
}

void ImGuiWrapper::createFramebuffers()
{
	ASSERT_TRUE(renderPass != VK_NULL_HANDLE);
	ASSERT_TRUE(framebuffers.empty());
	framebuffers.resize(gState.io.imageCount);
	framebufferExtent = gState.io.extent;
	for (size_t i = 0; i < gState.io.imageCount; i++)
	{
		VkFramebufferCreateInfo framebufferCreateInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
		framebufferCreateInfo.renderPass      = renderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments    = &gState.io.images[i].view;
		framebufferCreateInfo.width           = gState.io.extent.width;
		framebufferCreateInfo.height          = gState.io.extent.height;
		framebufferCreateInfo.layers          = 1;
		ASSERT_VULKAN(vkCreateFramebuffer(gState.device.logical, &framebufferCreateInfo, nullptr, &framebuffers[i]));
	}
}

void ImGuiWrapper::updatFramebuffers()
{
	if ((framebufferExtent.width == gState.io.extent.width && framebufferExtent.height == gState.io.extent.height) && !gState.swapchainRecreated())
	{
		return;
	}
	else
	{
		for (size_t i = 0; i < gState.io.imageCount; i++)
		{
			vkDestroyFramebuffer(gState.device.logical, framebuffers[i], nullptr);
		}
		framebuffers.clear();
		createFramebuffers();
	}
}

void ImGuiWrapper::init()
{
	createRenderPass();
	createFramebuffers();
	initImGui(renderPass);
}

void ImGuiWrapper::renderGui(UniversalCmdBuffer& cmdBuf)
{
	updatFramebuffers();
	std::vector<VkClearValue> clearValues = {{0.0f, 0.0f, 0.0f, 1.0f}};
	cmdBuf.startRenderPass(renderPass, framebuffers[gState.frame->frameIndex], clearValues);
	draw(cmdBuf);
	cmdBuf.endRenderPass();
}
}