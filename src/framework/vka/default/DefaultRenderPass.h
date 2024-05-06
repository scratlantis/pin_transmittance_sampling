#pragma once
#include "../combined_resources/CmdBuffer.h"
#include "../combined_resources/Image.h"
#include "../global_state.h"
#include "../resources/RasterizationPipeline.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>

namespace vka
{

class DefaultRenderPass
{
  public:
	DefaultRenderPass(Image *pOffscreenImage = nullptr);
	~DefaultRenderPass();

	//void renderGui(UniversalCmdBuffer cmdBuf);
	void init();
	void beginRender(UniversalCmdBuffer cmdBuf);
	void endRender(UniversalCmdBuffer cmdBuf);
	void destroy();
	RasterizationPipeline createPipeline(const RasterizationPipelineState pipelineState, uint32_t subpassIndex);
  private:
	void updatFramebuffers();
	void createRenderPass();
	void createFramebuffers();
	//void draw(UniversalCmdBuffer &cmdBuf);

	VkRenderPass               renderPass     = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> framebuffers;
	VkExtent2D                 framebufferExtent;
	Image                     *pOffscreenImage = nullptr;
};
}        // namespace vka