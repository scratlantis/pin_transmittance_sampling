#include "pt_interface.h"

void cmdTrace(CmdBuffer cmdBuf, Image target, TraceArgs args)
{
	Buffer                 camBuf, camInstBuf;
	std::hash<std::string> hasher;
	gState.dataCache->fetch(camBuf, hasher("cam_buf"));
	gState.dataCache->fetch(camInstBuf, hasher("cam_inst_buf"));
	cmdUpdateCamera(cmdBuf, camBuf, camInstBuf, args.cameraCI);

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

	cmdBarrier(cmdBuf, waitStage, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, accessFlags, VK_ACCESS_SHADER_READ_BIT);

	uvec2 invocationCount = uvec2(target->getExtent2D().width, target->getExtent2D().height);
	uvec2 denominator     = uvec2(args.subSampleMode, args.subSampleMode);
	invocationCount       = (invocationCount + denominator - uvec2(1U)) / denominator;

	ComputeCmd cmd(invocationCount, shaderPath + "pt.comp", {{"FORMAT1", getGLSLFormat(target->getFormat())}});
	bindCamera(cmd, camBuf, camInstBuf);
	bindScene(cmd, &args.resources.sceneData);
	bindScalarField(cmd, args.resources.mediumTexture, args.rayMarchStepSize);
	if (args.enableDebugging)
	{
		bindDebugModules(cmd, debugData, args.debugArgs);
		cmd.pushSubmodule(cVkaShaderLibPath + "pt_plot/generate_pt_state_histogram.glsl");
	}
	cmd.pushSIDebugHeader();
	bindCVSModule(cmd, cvsData, args.cvsArgs); // Binds pins
	cmd.pushLocal();
	cmd.pushDescriptor(target, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	cmd.pushDescriptor(args.resources.mediumInstanceBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(args.resources.mediumTlas);

	cmd.pushSpecializationConst(args.maxDepth);
	cmd.pushSpecializationConst(args.sampleCount);
	cmd.pushSpecializationConst(args.force_ray_marched_distance_sampling);
	cmd.pushSpecializationConst(args.force_ray_marched_transmittance_sampling_al);
	cmd.pushSpecializationConst(args.force_ray_marched_transmittance_sampling_env_map);


	if (args.skipGeometry)
	{
		cmd.pipelineDef.shaderDef.args.push_back({"SKIP_GEOMETRY", ""});
	}

	if (args.enableDebugging)
	{
		cmd.pipelineDef.shaderDef.args.push_back({"DEBUG", ""});
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
	} pc;
	pc.areaLightEmissionScale = args.areaLightEmissionScale;
	pc.envMapEmissionScale    = args.envMapEmissionScale;
	pc.subSampleMode          = args.subSampleMode;
	pc.executionID = *args.pExecutionCounter;
	pc.minDepth    = args.minDepth;
	pc.seed        = args.seed;
	pc.firstRandomBounce = args.firstRandomBounce;

	cmd.pushConstant(&pc, sizeof(PushStruct));

	IResourceCache *pCache = gState.cache;
	gState.cache = args.pTraceResourceCache;
	cmd.exec(cmdBuf);
	gState.cache = pCache;

	*args.pExecutionCounter = *args.pExecutionCounter + 1;
}