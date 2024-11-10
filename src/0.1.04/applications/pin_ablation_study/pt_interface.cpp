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
	/*if (args.enablePins)
	{
	    gState.dataCache->fetch(pinGridBuf, hasher("pinGridBuf"));
	    pins::cmdUpdatePinGrid(cmdBuf, pinGridBuf, args.mediumTexture, args.pinArgs.getUpdateArgs());
	    waitStage |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	    accessFlags |= VK_ACCESS_SHADER_WRITE_BIT;
	}*/

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
	// Pins
	/*if (args.enablePins)
	{
	    pins::cmdBindPins(cmd, pinGridBuf, args.pinArgs.getSampleArgs());
	}*/
	cmd.pushLocal();
	cmd.pushDescriptor(target, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	cmd.pushDescriptor(args.mediumInstanceBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(args.mediumTlas);

	cmd.pushSpecializationConst(args.maxDepth);
	cmd.pushSpecializationConst(args.debugArgs.minDepth);
	cmd.pushSpecializationConst(args.debugArgs.fixedSeed);
	cmd.pushSpecializationConst(args.debugArgs.firstRandomBounce);
	cmd.pipelineDef.shaderDef.args.push_back({"SAMPLE_COUNT", args.sampleCount});
	if (args.debugArgs.skipGeometry)
	{
		cmd.pipelineDef.shaderDef.args.push_back({"SKIP_GEOMETRY", ""});
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