#include "pt_interface.h"

void cmdTrace(CmdBuffer cmdBuf, Image target, TraceArgs args)
{
	Buffer                 camBuf, camInstBuf;
	std::hash<std::string> hasher;
	gState.dataCache->fetch(camBuf, hasher("cam_buf"));
	gState.dataCache->fetch(camInstBuf, hasher("cam_inst_buf"));
	cmdUpdateCamera(cmdBuf, camBuf, camInstBuf, args.sceneParams.cameraCI);

	TraceDebugData       debugData;
	VkPipelineStageFlags waitStage   = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkAccessFlags        accessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
	if (args.enableDebugging)
	{
		debugData = cmdPrepareDebugData(cmdBuf, args);
	}
	
	CVSData cvsData = cmdPrepareCVSData(cmdBuf, args); // Commputes pins
	if (args.cvsArgs.pinArgs.type != PinType::NONE)
	{
		waitStage  |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		accessFlags |= VK_ACCESS_SHADER_WRITE_BIT;
	}
	Buffer scalarFieldUniform = cmdGetScalarFieldUniform(cmdBuf, gState.frame->stack, args.sceneParams.densityScale, args.sceneParams.minDensity, args.sceneParams.maxDensity);
	cmdBarrier(cmdBuf, waitStage, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, accessFlags, VK_ACCESS_SHADER_READ_BIT);

	uvec2 invocationCount = uvec2(target->getExtent2D().width, target->getExtent2D().height);
	uvec2 denominator     = uvec2(args.config.subSampleMode, args.config.subSampleMode);
	invocationCount       = (invocationCount + denominator - uvec2(1U)) / denominator;

	ComputeCmd cmd(invocationCount, shaderPath + "pt.comp", {{"FORMAT1", getGLSLFormat(target->getFormat())}});
	bindCamera(cmd, camBuf, camInstBuf);
	bindScene(cmd, &args.resources.sceneData);
	bindScalarField(cmd, args.resources.mediumTexture, args.config.rayMarchStepSize, scalarFieldUniform);
	if (args.enableDebugging)
	{
		bindDebugModules(cmd, debugData, args.debugArgs);
		cmd.pushSubmodule(cVkaShaderLibPath + "pt_plot/generate_pt_state_histogram.glsl");
	}
	cmd.pushSIDebugHeader();
	bindCVSModule(cmd, cvsData, args); // Binds pins
	cmd.pushLocal();
	cmd.pushDescriptor(target, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	cmd.pushDescriptor(args.resources.mediumInstanceBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(args.resources.mediumTlas);

	cmd.pushSpecializationConst(args.config.maxDepth);
	cmd.pushSpecializationConst(args.config.sampleCount);
	cmd.pushSpecializationConst(args.config.force_ray_marched_distance_sampling);
	cmd.pushSpecializationConst(args.config.force_ray_marched_transmittance_sampling_al);
	cmd.pushSpecializationConst(args.config.force_ray_marched_transmittance_sampling_env_map);


	if (args.sceneParams.skipGeometry)
	{
		cmd.pipelineDef.shaderDef.args.push_back({"SKIP_GEOMETRY", ""});
	}

	if (args.enableDebugging)
	{
		cmd.pipelineDef.shaderDef.args.push_back({"DEBUG", ""});
	}
	if (args.sceneParams.scatterFunc == 1)
	{
		cmd.pipelineDef.shaderDef.args.push_back({"GREENSTEIN", ""});
	}

	struct PushStruct
	{
		float areaLightEmissionScale;
		float envMapEmissionScale;
		uint32_t subSampleMode;
		uint32_t executionID;

		uint32_t minDepth;
		uint32_t seed;
		uint32_t firstRandomBounce;
		float    g;
	} pc;
	pc.areaLightEmissionScale = args.sceneParams.areaLightEmissionScale;
	pc.envMapEmissionScale    = args.sceneParams.envMapEmissionScale;
	pc.subSampleMode          = args.config.subSampleMode;
	pc.executionID = *args.pExecutionCounter;
	pc.minDepth    = args.config.minDepth;
	pc.seed        = args.config.seed;
	pc.firstRandomBounce = args.config.firstRandomBounce;
	pc.g           = args.sceneParams.scatterFuncG;

	cmd.pushConstant(&pc, sizeof(PushStruct));

	IResourceCache *pCache = gState.cache;
	gState.cache = args.pTraceResourceCache;
	cmd.exec(cmdBuf);
	gState.cache = pCache;

	*args.pExecutionCounter = *args.pExecutionCounter + 1;
}