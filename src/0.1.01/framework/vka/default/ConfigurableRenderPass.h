#pragma once
#include "../combined_resources/CmdBuffer.h"
#include "../combined_resources/Image.h"
#include "../global_state.h"
#include "../resources/RasterizationPipeline.h"
#include "RenderPass.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>

namespace vka
{

struct ConfigurableRenderPassCI
{
	Image                     *pDepthImage;
	std::vector<Image *>       pColorAttachments;
	std::vector<bool>          colorClear;
	std::vector<VkClearValue>  colorClearValues;
	std::vector<VkImageLayout> colorInitialLayout;
	std::vector<VkImageLayout> colorTargetLayout;
	Rect2D<float>              relRenderArea;
};

// Still only one subpass
class ConfigurableRenderPass : public RenderPass
{
  public:
	ConfigurableRenderPass(){};
	ConfigurableRenderPass(ConfigurableRenderPassCI ci);
	~ConfigurableRenderPass();

	// void renderGui(UniversalCmdBuffer cmdBuf);
	void                  init();
	void                  beginRender(UniversalCmdBuffer &cmdBuf);
	void                  endRender(UniversalCmdBuffer &cmdBuf);
	void                  destroy();
	RasterizationPipeline createPipeline(const RasterizationPipelineState pipelineState, uint32_t subpassIndex) const;

  private:
	void updatFramebuffers();
	void createRenderPass();
	void createFramebuffers();

	// Local data
	VkRenderPass               renderPass = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> framebuffers;
	VkExtent2D                 framebufferExtent;

	// Conf
	Image                     *pDepthImage;
	std::vector<Image *>       pColorAttachments;
	std::vector<bool>          colorClear;
	std::vector<VkClearValue>  colorClearValues;
	std::vector<VkImageLayout> colorInitialLayout;
	std::vector<VkImageLayout> colorTargetLayout;
	Rect2D<float>              relRenderArea;


};
}        // namespace vka