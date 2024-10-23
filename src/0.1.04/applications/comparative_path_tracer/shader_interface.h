#include "config.h"
using namespace default_scene;

#include "shaders/interface_structs.glsl"



struct TraceDebugArgs
{
	vec2                   pixelPos;
	bool                   enablePlot;
	uint32_t               maxPlotCount;
	uint32_t               maxPlotValueCount;
	bool                   enableHistogram;
	bool                   resetHistogram;
	uint32_t               histogramDataOffset;
	uint32_t               maxHistCount;
	uint32_t               maxHistValueCount;
	bool                   enablePtPlot;
	pt_plot::PtPlotOptions ptPlotOptions;
};

struct TraceArgs
{
	uint32_t        sampleCount;
	uint32_t        maxDepth;
	float           rayMarchStepSize;
	CameraCI        cameraCI;
	USceneData      sceneData;
	Buffer          mediumInstanceBuffer;
	Image           mediumTexture;
	bool            enableDebugging;
	TraceDebugArgs	debugArgs;
};


void cmdTrace(CmdBuffer cmdBuf, Image target, TraceArgs args)
{
	Buffer camBuf, camInstBuf;
	std::hash<std::string> hasher;
	gState.dataCache->fetch(camBuf, hasher("cam_buf"));
	gState.dataCache->fetch(camInstBuf, hasher("cam_inst_buf"));
	cmdUpdateCamera(cmdBuf, camBuf, camInstBuf, args.cameraCI);
	// Plot resources:
	Buffer selectionBuf, plotBuf, plotDataBuf, plotCountBuf;
	// Histogram resources:
	Buffer histBuf, histDataBuf, histCountBuf;
	// Shader state resources:
	Buffer indirectBounceBuf, directRayBuf, stateBuf;
	// Pt Plot resources:
	Buffer ptPlotOptionsBuf, ptPlotBuf;
	bool   histogramBuffersInitialized = true;
	if (args.enableDebugging)
	{
		// Ivocation selection
		{
			gState.dataCache->fetch(selectionBuf, hasher("selection"));
			shader_debug::cmdSelectInvocation(cmdBuf, selectionBuf, args.debugArgs.pixelPos);
		}
		// Path tracing shader state
		{
			gState.feedbackDataCache->fetch(indirectBounceBuf, hasher("indirectBounce"));
			gState.feedbackDataCache->fetch(directRayBuf, hasher("directRay"));
			gState.feedbackDataCache->fetch(stateBuf, hasher("state"));
			shader_debug::cmdResetPtShaderState(cmdBuf, indirectBounceBuf, directRayBuf, stateBuf, args.maxDepth, maxIndirectRaysPerBounce);
		}
		// Y list Plot
		if (args.debugArgs.enablePlot)
		{
			gState.feedbackDataCache->fetch(plotBuf, hasher("plot"));
			gState.feedbackDataCache->fetch(plotDataBuf, hasher("plotData"));
			gState.feedbackDataCache->fetch(plotCountBuf, hasher("plotCount"));
			shader_plot::cmdResetYListPlot(cmdBuf, plotBuf, plotDataBuf, plotCountBuf, args.debugArgs.maxPlotCount, args.debugArgs.maxPlotValueCount);
		}
		// Historgram resources
		if (args.debugArgs.enableHistogram)
		{
			histogramBuffersInitialized      = gState.feedbackDataCache->fetch(histBuf, hasher("hist")) && histogramBuffersInitialized;
			histogramBuffersInitialized      = gState.feedbackDataCache->fetch(histDataBuf, hasher("histData")) && histogramBuffersInitialized;
			histogramBuffersInitialized      = gState.feedbackDataCache->fetch(histCountBuf, hasher("histCount")) && histogramBuffersInitialized;

			if (args.debugArgs.resetHistogram)
			{
				histogramBuffersInitialized = true;
				shader_plot::cmdResetHistogram(cmdBuf, histBuf, histDataBuf, histCountBuf, args.debugArgs.maxHistCount, args.debugArgs.maxHistValueCount);
			}
		}
		if (args.debugArgs.enablePtPlot)
		{
			gState.feedbackDataCache->fetch(ptPlotOptionsBuf, hasher("ptPlotOptions"));
			gState.feedbackDataCache->fetch(ptPlotBuf, hasher("ptPlot"));
			pt_plot::cmdConfigurePtPlot(cmdBuf, ptPlotOptionsBuf, ptPlotBuf, args.debugArgs.ptPlotOptions);

		}
	}
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	ComputeCmd cmd(target->getExtent2D(), shaderPath + "pt.comp", {{"FORMAT1", getGLSLFormat(target->getFormat())}});
	bindCamera(cmd, camBuf, camInstBuf);
#ifdef RAY_TRACING_SUPPORT
	bindScene(cmd, &args.sceneData);
#else
	bindMockScene(cmd);
#endif
	bindScalarField(cmd, args.mediumTexture, args.rayMarchStepSize);
	if (args.enableDebugging)
	{
		cmd.pipelineDef.shaderDef.args.push_back({"DEBUG", ""});
		shader_debug::bindInvocationSelection(cmd, selectionBuf);
		shader_debug::bindPtShaderState(cmd, indirectBounceBuf, directRayBuf, stateBuf);
		if (args.debugArgs.enablePlot)
		{
			shader_plot::bindYListPlot(cmd, plotBuf, plotDataBuf, plotCountBuf);
		}
		if (args.debugArgs.enableHistogram)
		{
			VKA_ASSERT(histogramBuffersInitialized);
			if (args.debugArgs.histogramDataOffset > 0)
			{
				BufferRange range = histDataBuf->getRange();
				range.offset += args.debugArgs.histogramDataOffset;
				histDataBuf = histDataBuf->getSubBuffer(range);
			}
			shader_plot::bindHistogram(cmd, histBuf, histDataBuf, histCountBuf);
		}
		if (args.debugArgs.enablePtPlot)
		{
			pt_plot::bindPtPlot(cmd, ptPlotOptionsBuf, ptPlotBuf);
		}
	}

	cmd.startLocalBindings();
	cmd.pushDescriptor(target, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	cmd.pushDescriptor(args.mediumInstanceBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	cmd.pipelineDef.shaderDef.args.push_back({"MAX_BOUNCES", args.maxDepth});
	cmd.pipelineDef.shaderDef.args.push_back({"SAMPLE_COUNT", args.sampleCount});
	cmd.exec(cmdBuf);
}