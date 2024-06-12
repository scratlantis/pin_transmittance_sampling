#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>
#include <vka/state_objects/global_state.h>
#include <vka/interface/types.h>

namespace vka
{

class ImGuiWrapper
{
  public:
	ImGuiWrapper(){};
	~ImGuiWrapper(){};

	void         vkaImGuiInit(VkRenderPass renderPass, uint32_t subpassIdx);
	void         vkaImGuiUpload(VkaCommandBuffer cmdBuf);
	void         vkaImGuiFreeStaging();
	void         vkaImGuiRender(VkaCommandBuffer cmdBuf);
	void         vkaImGuiNewFrame();
	void         vkaImGuiDestroy();

  private:
	VkDescriptorPool           descriptorPool = VK_NULL_HANDLE;
};
}        // namespace vka