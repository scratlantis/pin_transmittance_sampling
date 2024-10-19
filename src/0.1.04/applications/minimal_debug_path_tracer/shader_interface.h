#include "config.h"
using namespace default_scene;

#include "shaders/interface_structs.glsl"

struct TraceDebugArgs
{
	vec2 pixelPos;
	uint32_t maxPlotCount;
	uint32_t maxPlotValueCount;
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
	TraceDebugArgs *pDebugArgs;
};


void cmdTrace(CmdBuffer cmdBuf, Image target, TraceArgs args)
{
	Buffer camBuf, camInstBuf;
	std::hash<std::string> hasher;
	gState.dataCache->fetch(camBuf, hasher("cam_buf"));
	gState.dataCache->fetch(camInstBuf, hasher("cam_inst_buf"));
	cmdUpdateCamera(cmdBuf, camBuf, camInstBuf, args.cameraCI);

	Buffer selectionBuf, plotBuf, plotDataBuf, plotCountBuf;
	if (args.pDebugArgs)
	{
		gState.dataCache->fetch(selectionBuf, hasher("selection"));
		shader_debug::cmdSelectInvocation(cmdBuf, selectionBuf, args.pDebugArgs->pixelPos);

		gState.feedbackDataCache->fetch(plotBuf, hasher("plot"));
		gState.feedbackDataCache->fetch(plotDataBuf, hasher("plotData"));
		gState.feedbackDataCache->fetch(plotCountBuf, hasher("plotCount"));
		shader_plot::cmdResetYListPlot(cmdBuf, plotBuf, plotDataBuf, plotCountBuf, args.pDebugArgs->maxPlotCount, args.pDebugArgs->maxPlotValueCount);
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
	if (args.pDebugArgs)
	{
		shader_debug::bindInvocationSelection(cmd, selectionBuf);
		shader_plot::bindYListPlot(cmd, plotBuf, plotDataBuf, plotCountBuf);
		cmd.pipelineDef.shaderDef.args.push_back({"DEBUG", ""});
	}

	cmd.startLocalBindings();
	cmd.pushDescriptor(target, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	cmd.pushDescriptor(args.mediumInstanceBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	cmd.pipelineDef.shaderDef.args.push_back({"MAX_BOUNCES", args.maxDepth});
	cmd.pipelineDef.shaderDef.args.push_back({"SAMPLE_COUNT", args.sampleCount});
	cmd.exec(cmdBuf);
}