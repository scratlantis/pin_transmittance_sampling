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

	ComputeCmd cmd(target->getExtent2D(), shaderPath + "pt.comp", {{"FORMAT1", getGLSLFormat(target->getFormat())}});
	bindCamera(cmd, camBuf, camInstBuf);
	bindScene(cmd, &args.sceneData);
	bindScalarField(cmd, args.mediumTexture, args.rayMarchStepSize);
	if (args.enableDebugging)
	{
		bindDebugModules(cmd, debugData, args.debugArgs);
		cmd.pushSubmodule(cVkaShaderLibPath + "pt_plot/generate_pt_state_histogram.glsl");
	}
	cmd.pushSIDebugHeader();
	bindCVSModule(cmd, cvsData, args.cvsArgs); // Binds pins
	cmd.pushLocal();
	cmd.pushDescriptor(target, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	cmd.pushDescriptor(args.mediumInstanceBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(args.mediumTlas);

	cmd.pushSpecializationConst(args.maxDepth);
	cmd.pushSpecializationConst(args.sampleCount);
	cmd.pushSpecializationConst(args.minDepth);
	cmd.pushSpecializationConst(args.fixedSeed);
	cmd.pushSpecializationConst(args.firstRandomBounce);

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
	} pc;
	pc.areaLightEmissionScale = args.areaLightEmissionScale;
	pc.envMapEmissionScale    = args.envMapEmissionScale;
	cmd.pushConstant(&pc, sizeof(PushStruct));
	cmd.exec(cmdBuf);
}