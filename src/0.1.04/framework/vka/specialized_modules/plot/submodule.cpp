#include "submodule.h"

namespace vka
{
namespace shader_plot
{
void bindYListPlot(ComputeCmd &cmd, Buffer plotBuf, Buffer plotDataBuf, Buffer plotCountBuf)
{
	cmd.pushSubmodule(cVkaShaderModulePath + "plot/y_list_plot_smd.glsl");
	cmd.pushDescriptor(plotBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(plotDataBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(plotCountBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
}
void cmdResetYListPlot(CmdBuffer cmdBuf, Buffer plotBuf, Buffer plotDataBuf, Buffer plotCountBuf, uint32_t maxPlotCount, uint32_t maxPlotValueCount)
{
	plotBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	plotDataBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	plotCountBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	plotBuf->changeSize(maxPlotCount * sizeof(GLSLYListPlot));
	plotDataBuf->changeSize(maxPlotValueCount * sizeof(float));
	plotCountBuf->changeSize(sizeof(uint32_t));

	plotBuf->recreate();
	plotDataBuf->recreate();
	plotCountBuf->recreate();

	cmdZeroBuffer(cmdBuf, plotBuf);
	cmdZeroBuffer(cmdBuf, plotDataBuf);
	cmdZeroBuffer(cmdBuf, plotCountBuf);
}
}        // namespace shader_plot
}		// namespace vka