#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
//#include "../global_state.h"
//#include <imgui.h>
//#include <imgui_impl_vulkan.h>
//#include <imgui_impl_glfw.h>
//
//namespace vka
//{
//
//class ImguiRenderSubpass : public Subpass
//{
//  public:
//	ImguiRenderSubpass();
//	~ImguiRenderSubpass();
//
//	VkSubpassDependency  getSubpassDependency();
//	VkSubpassDescription getSubpassDescription();
//
//  private:
//	// GBuffer
//	std::vector<VkAttachmentReference> colorAttachmentReferenceGeometryPass =
//	    {
//	        {DEFERRED_ATTACHMENT_ID_SWAPCHAIN, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
//};
//
//inline ImguiRenderSubpass::ImguiRenderSubpass()
//{
//}
//
//inline ImguiRenderSubpass::~ImguiRenderSubpass()
//{
//}
//
//inline VkSubpassDependency ImguiRenderSubpass::getSubpassDependency()
//{
//}
//
//inline VkSubpassDescription ImguiRenderSubpass::getSubpassDescription()
//{
//	VkSubpassDescription subpassDescriptionGeometryPass = {};
//	subpassDescriptionGeometryPass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
//	subpassDescriptionGeometryPass.colorAttachmentCount = colorAttachmentReferenceGeometryPass.size();
//	subpassDescriptionGeometryPass.pColorAttachments    = colorAttachmentReferenceGeometryPass.data();
//	return subpassDescriptionGeometryPass;
//}
//
//
//class ImGuiWrapper:
//{
//  public:
//	ImGuiWrapper();
//	~ImGuiWrapper();
//
//	void init(VkRenderPass renderPass);
//
//	void     destroyPipeline();
//	void     createPipeline(VkRenderPass renderPass);
//	void     recordCmds(VkCommandBuffer &cmdBuf, DeferredParams params);
//	void     render_gui();
//
//  private:
//	void addGVar(GVar *gv);
//	void addGVars(GVar_Cat category);
//	void buildGui();
//
//	VkPipeline       pipeline;
//	VkPipelineLayout pipelineLayout;
//	VkDescriptorPool descriptorPool;
//};
//
//
//
//
//ImGuiWrapper::ImGuiWrapper()
//{
//
//	// Init stuff
//	IMGUI_CHECKVERSION();
//	ImGui::CreateContext();
//	ImGuiIO &io = ImGui::GetIO();
//	(void) io;
//
//	float size_pixels = 18;
//	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/Roboto-Medium.ttf", size_pixels);
//	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/Cousine-Regular.ttf", size_pixels);
//	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/DroidSans.ttf", size_pixels);
//	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/Karla-Regular.ttf", size_pixels);
//	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/ProggyClean.ttf", size_pixels);
//	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/ProggyTiny.ttf", size_pixels);
//
//	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10);
//	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10);
//	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
//	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
//
//	// Setup Dear ImGui style
//	ImGui::StyleColorsDark();
//	// ImGui::StyleColorsClassic();
//	gState.io.getWindow()->initGui();
//
//	// Create Descriptor Pool
//	VkDescriptorPoolSize gui_pool_sizes[] =
//	    {
//	        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
//	        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
//	        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
//	        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
//	        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
//	        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
//	        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
//	        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
//	        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
//	        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
//	        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};
//
//	VkDescriptorPoolCreateInfo gui_pool_info = {};
//	gui_pool_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//	gui_pool_info.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
//	gui_pool_info.maxSets                    = 1000 * IM_ARRAYSIZE(gui_pool_sizes);
//	gui_pool_info.poolSizeCount              = (uint32_t) IM_ARRAYSIZE(gui_pool_sizes);
//	gui_pool_info.pPoolSizes                 = gui_pool_sizes;
//
//	VkResult result = vkCreateDescriptorPool(gState.device.logical, &gui_pool_info, nullptr, &gui_descriptor_pool);
//
//	if (result != VK_SUCCESS)
//	{
//		throw std::runtime_error("Failed to create a gui descriptor pool!");
//	}
//}
//
//void ImGuiWrapper::destroy()
//{
//	// clean up of GUI stuff
//	ImGui_ImplVulkan_Shutdown();
//}
//
//ImGuiWrapper::~ImGuiWrapper()
//{
//}
//
//
//
//void ImGuiWrapper::init(VkRenderPass renderPass)
//{
//	ImGui_ImplVulkan_InitInfo init_info = {};
//	init_info.Instance                  = gState.device.instance;
//	init_info.PhysicalDevice            = gState.device.physical;
//	init_info.Device                    = gState.device.logical;
//	init_info.QueueFamily               = gState.device.universalQueueFamily;
//	init_info.Queue                     = gState.device.universalQueues[0];
//	init_info.DescriptorPool            = descriptorPool;
//	init_info.PipelineCache             = VK_NULL_HANDLE;        // we do not need those
//	init_info.MinImageCount             = 2;
//	init_info.ImageCount                = gState.io.imageCount;
//	init_info.Allocator                 = VK_NULL_HANDLE;
//	init_info.CheckVkResultFn           = VK_NULL_HANDLE;
//	init_info.MSAASamples               = VK_SAMPLE_COUNT_1_BIT;
//	ImGui_ImplVulkan_Init(&init_info, renderPass);
//
//
//
//
//	// Upload stuff
//	VkCommandBuffer command_buffer = rm->cmdBufStartSingleTime();
//	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
//	rm->endSubmitWait(command_buffer);
//	ImGui_ImplVulkan_DestroyFontUploadObjects();
//}
//
//// Helper to display a little (?) mark which shows a tooltip when hovered.
//// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
//static void HelpMarker(const char *desc)
//{
//	ImGui::TextDisabled("(?)");
//	if (ImGui::IsItemHovered())
//	{
//		ImGui::BeginTooltip();
//		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
//		ImGui::TextUnformatted(desc);
//		ImGui::PopTextWrapPos();
//		ImGui::EndTooltip();
//	}
//}
//
//void ImGuiWrapper::addGVar(GVar *gv)
//{
//	std::stringstream ss;
//	switch (gv->type)
//	{
//		case GVAR_EVENT:
//			gv->val.v_bool = ImGui::Button(gv->name.c_str());
//			break;
//		case GVAR_BOOL:
//			ImGui::Checkbox(gv->name.c_str(), &gv->val.v_bool);
//			break;
//		case GVAR_FLOAT:
//			ImGui::InputScalar(gv->name.c_str(), ImGuiDataType_Float, &gv->val.v_float);
//			break;
//		case GVAR_UINT:
//			ImGui::InputScalar(gv->name.c_str(), ImGuiDataType_U32, &gv->val.v_uint);
//			break;
//		case GVAR_INT:
//			ImGui::InputInt(gv->name.c_str(), &gv->val.v_int);
//			break;
//		case GVAR_VEC3:
//			ImGui::InputFloat3(gv->name.c_str(), gv->val.v_vec3);
//			break;
//		case GVAR_DISPLAY_VALUE:
//			ImGui::Text(gv->name.c_str(), gv->val.v_float);
//			break;
//		case GVAR_ENUM:
//			for (size_t i = 0; i < gv->enumVal.size(); i++)
//			{
//				ss << gv->enumVal[i] << '\0';
//			}
//			ImGui::Combo(gv->name.c_str(), &gv->val.v_int, ss.str().c_str(), 5);
//			break;
//		default:
//			break;
//	}
//}
//
//void ImGuiWrapper::addGVars(GVar_Cat category)
//{
//	for (uint32_t i = 0; i < gVars.size(); i++)
//	{
//		if (gVars[i]->cat == category)
//		{
//			addGVar(gVars[i]);
//		}
//	}
//}
//
//void ImGuiWrapper::buildGui()
//{
//	ImGui::Begin("Gui");
//
//	if (ImGui::CollapsingHeader("Application"))
//	{
//		addGVars(GVAR_APPLICATION);
//	}
//
//	if (ImGui::CollapsingHeader("Framework"))
//	{
//		addGVars(GVAR_FRAMEWORK);
//	}
//
//	ImGui::End();
//}
//
//
//void ImGuiWrapper::recordCmds(VkCommandBuffer &cmdBuf, DeferredParams params)
//{
//	// render_gui();
//	ImGui_ImplGlfw_NewFrame();
//
//	ImGui::NewFrame();
//
//	// ImGui::ShowDemoWindow();
//	buildGui();
//	ImGui::Render();
//	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuf);
//}
//void ImguiRenderer::init()
//{
//	createRenderAttachments();
//	createRenderPass();
//	imguiRenderStage->createPipeline(renderPass);
//	createFramebuffers();
//}
//
//
//void ImguiRenderer::createRenderPass()
//{
//	std::vector<VkAttachmentDescription> attachementDescriptions;
//
//	for (uint32_t i = 0; i < attachments.size(); i++)
//	{
//		VkAttachmentDescription attachment = attachments[i]->getAttachmentDescriptor();
//		attachment.loadOp        = VK_ATTACHMENT_LOAD_OP_LOAD;
//		attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//		attachementDescriptions.push_back(attachment);
//	}
//
//	std::vector<Subpass *> subpasses;
//
//	subpasses.push_back(imguiRenderStage->getSubpass());
//
//	device->createRenderPassExt(attachementDescriptions, subpasses, renderPass);
//}
//
//void ImguiRenderer::createFramebuffers()
//{
//	framebuffers.resize(device->swapchainSize);
//
//	for (size_t i = 0; i < device->swapchainSize; i++)
//	{
//		std::vector<VkImageView> attachmentViews;
//		for (uint32_t j = 0; j < attachments.size(); j++)
//		{
//			attachmentViews.push_back(attachments[j]->getImageView(i));
//		}
//
//		VkFramebufferCreateInfo framebufferCreateInfo;
//		framebufferCreateInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//		framebufferCreateInfo.pNext           = nullptr;
//		framebufferCreateInfo.flags           = 0;
//		framebufferCreateInfo.renderPass      = renderPass;
//		framebufferCreateInfo.attachmentCount = attachmentViews.size();
//		framebufferCreateInfo.pAttachments    = attachmentViews.data();
//		framebufferCreateInfo.width           = device->width;
//		framebufferCreateInfo.height          = device->height;
//		framebufferCreateInfo.layers          = 1;
//		framebuffers[i]                       = device->createFramebuffer(&framebufferCreateInfo);
//	}
//}
//
//void ImguiRenderer::createRenderAttachments()
//{
//	attachments.resize(1);
//
//	// Create attachments
//	this->attachments[DEFERRED_ATTACHMENT_ID_SWAPCHAIN] = new SwapchainAttachment(device);
//}
//
//void ImguiRenderer::recordRender(VkCommandBuffer cmdBuf, View &view)
//{
//	recordStartRenderPass(cmdBuf);
//
//	DeferredParams params{};
//	params.geom        = view.geom;
//	params.lights      = view.pointLights;
//	params.boid_number = view.boids_number;
//	imguiRenderStage->recordCmds(cmdBuf, params);
//	vkCmdEndRenderPass(cmdBuf);
//}
//
//void ImguiRenderer::recordStartRenderPass(VkCommandBuffer cmdBuf)
//{
//	std::vector<VkClearValue> clearValues;
//	for (uint32_t i = 0; i < attachments.size(); i++)
//	{
//		clearValues.push_back(attachments[i]->getClearValue());
//	}
//
//	uint32_t idx = device->swapchainIndex;
//
//	VkRenderPassBeginInfo renderPassBeginInfo;
//	renderPassBeginInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//	renderPassBeginInfo.pNext             = nullptr;
//	renderPassBeginInfo.renderPass        = renderPass;
//	renderPassBeginInfo.framebuffer       = framebuffers[idx];
//	renderPassBeginInfo.renderArea.offset = {0, 0};
//	renderPassBeginInfo.renderArea.extent = {device->width, device->height};
//	renderPassBeginInfo.clearValueCount   = clearValues.size();
//	renderPassBeginInfo.pClearValues      = clearValues.data();
//
//	vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
//	VkViewport viewport;
//	viewport.x        = 0.0f;
//	viewport.y        = 0.0f;
//	viewport.width    = device->width;
//	viewport.height   = device->height;
//	viewport.minDepth = 0.0f;
//	viewport.maxDepth = 1.0f;
//	vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
//	VkRect2D scissor;
//	scissor.offset = {0, 0};
//	scissor.extent = {device->width, device->height};
//	vkCmdSetScissor(cmdBuf, 0, 1, &scissor);
//}
//
//ImguiRenderer::~ImguiRenderer()
//{
//}
//
//
//}