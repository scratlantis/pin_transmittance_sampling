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

class RenderPass
{
  public:
	RenderPass(){};
	virtual void                  init()                                                                                      = 0;
	virtual void                  beginRender(UniversalCmdBuffer &cmdBuf)                                                     = 0;
	virtual void                  endRender(UniversalCmdBuffer &cmdBuf)                                                       = 0;
	virtual void                  destroy()                                                                                   = 0;
	virtual RasterizationPipeline createPipeline(const RasterizationPipelineState pipelineState, uint32_t subpassIndex) const = 0;
	~RenderPass(){};

  private:
};
}        // namespace vka