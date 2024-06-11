#pragma once
#include "VkaImGuiWrapper.h"
#include <vka/state_objects/global_var.h>
namespace vka
{

class GvarGui : public ImGuiWrapper
{
  public:
	GvarGui(){};
	void newFrame()
	{
		vkaImGuiNewFrame();
	}
	void create(VkRenderPass renderPass, uint32_t subpassIdx)
	{
		configure();
		vkaImGuiInit(renderPass, subpassIdx);
	}
	void upload(VkaCommandBuffer cmdBuf)
	{
		vkaImGuiUpload(cmdBuf);
	}
	void freeStaging()
	{
		vkaImGuiFreeStaging();
	}
	void render(VkaCommandBuffer cmdBuf)
	{
		buildGui();
		vkaImGuiRender(cmdBuf);
	}
	void destroy()
	{
		vkaImGuiDestroy();
	}
  private:
	void configure();
	void buildGui();
	void addGVar(GVar *gv);
	void addGVars(GVar_Cat category);
};
}		// namespace vka
