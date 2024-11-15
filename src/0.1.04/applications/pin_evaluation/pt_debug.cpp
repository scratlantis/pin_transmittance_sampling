#include "pt_debug.h"
#include "pt_interface.h"
TraceDebugData cmdPrepareDebugData(CmdBuffer cmdBuf, TraceArgs args)
{
	TraceDebugData data{};
	std::hash<std::string> hasher;
	// Ivocation selection
	{
		gState.dataCache->fetch(data.selectionBuf, hasher("selection"));
		shader_debug::cmdSelectInvocation(cmdBuf, data.selectionBuf, args.debugArgs.pixelPos);
	}
	// Path tracing shader state
	{
		gState.feedbackDataCache->fetch(data.indirectBounceBuf, hasher("indirectBounce"));
		gState.feedbackDataCache->fetch(data.directRayBuf, hasher("directRay"));
		gState.feedbackDataCache->fetch(data.stateBuf, hasher("state"));
		shader_debug::cmdResetPtShaderState(cmdBuf, data.indirectBounceBuf, data.directRayBuf, data.stateBuf, args.config.maxDepth, maxIndirectRaysPerBounce);
	}
	// Y list Plot
	if (args.debugArgs.enablePlot)
	{
		gState.feedbackDataCache->fetch(data.plotBuf, hasher("plot"));
		gState.feedbackDataCache->fetch(data.plotDataBuf, hasher("plotData"));
		gState.feedbackDataCache->fetch(data.plotCountBuf, hasher("plotCount"));
		shader_plot::cmdResetYListPlot(cmdBuf, data.plotBuf, data.plotDataBuf, data.plotCountBuf, args.debugArgs.maxPlotCount, args.debugArgs.maxPlotValueCount);
	}
	// Historgram resources
	if (args.debugArgs.enableHistogram)
	{
		bool histogramBuffersInitialized = gState.feedbackDataCache->fetch(data.histBuf, hasher("hist"));
		histogramBuffersInitialized = gState.feedbackDataCache->fetch(data.histDataBuf, hasher("histData")) && histogramBuffersInitialized;
		histogramBuffersInitialized = gState.feedbackDataCache->fetch(data.histCountBuf, hasher("histCount")) && histogramBuffersInitialized;

		if (args.debugArgs.resetHistogram)
		{
			histogramBuffersInitialized = true;
			shader_plot::cmdResetHistogram(cmdBuf, data.histBuf, data.histDataBuf, data.histCountBuf, args.debugArgs.maxHistCount, args.debugArgs.maxHistValueCount);
		}
	}
	if (args.debugArgs.enablePtPlot)
	{
		gState.feedbackDataCache->fetch(data.ptPlotOptionsBuf, hasher("ptPlotOptions"));
		gState.feedbackDataCache->fetch(data.ptPlotBuf, hasher("ptPlot"));
		pt_plot::cmdConfigurePtPlot(cmdBuf, data.ptPlotOptionsBuf, data.ptPlotBuf, args.debugArgs.ptPlotOptions);
	}
	return data;
}

void bindDebugModules(ComputeCmd &cmd, TraceDebugData data, TraceDebugArgs args)
{
	cmd.pipelineDef.shaderDef.args.push_back({"DEBUG", ""});
	shader_debug::bindInvocationSelection(cmd, data.selectionBuf);
	shader_debug::bindPtShaderState(cmd, data.indirectBounceBuf, data.directRayBuf, data.stateBuf);
	if (args.enablePlot)
	{
		shader_plot::bindYListPlot(cmd, data.plotBuf, data.plotDataBuf, data.plotCountBuf);
	}
	if (args.enableHistogram)
	{
		if (args.histogramDataOffset > 0)
		{
			BufferRange range = data.histDataBuf->getRange();
			range.offset += args.histogramDataOffset;
			data.histDataBuf = data.histDataBuf->getSubBuffer(range);
		}
		shader_plot::bindHistogram(cmd, data.histBuf, data.histDataBuf, data.histCountBuf);
	}
	if (args.enablePtPlot)
	{
		pt_plot::bindPtPlot(cmd, data.ptPlotOptionsBuf, data.ptPlotBuf);
	}
}