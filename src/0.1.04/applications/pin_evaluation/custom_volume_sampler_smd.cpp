#include "custom_volume_sampler_smd.h"
#include "shaders/pins/interface_structs.glsl"
#include "pt_interface.h"

CVSData cmdPrepareCVSData(CmdBuffer cmdBuf, const TraceArgs &args)
{
	const CVSArgs &cvsArgs = args.cvsArgs;
	std::hash<std::string> hasher;
	CVSData data{};
	if (cvsArgs.pinArgs.type == PinType::NONE)
	{
		return data;
	}
	uint32_t pinStructSize = 0;
	switch (cvsArgs.pinArgs.type)
	{
		case PinType::V1:
			pinStructSize = sizeof(GLSLPinCacheEntryV1);
			break;
		case PinType::V2:
			pinStructSize = sizeof(GLSLPinCacheEntryV2);
			break;
		case PinType::V3:
			pinStructSize = sizeof(GLSLPinCacheEntryV3);
			break;
	}

	//uint32_t pinGridCellCount = cubed(cvsArgs.pinGridExtent.positionGridSize) * squared(cvsArgs.pinGridExtent.directionGridSize);
	uint32_t pinGridCellCount = cubed(cvsArgs.pinGridExtent.positionGridSize) * squared(cvsArgs.pinGridExtent.directionGridSize) * 4;
	uint32_t pinGridMemorySize = pinGridCellCount * pinStructSize;
	VKA_ASSERT(pinGridMemorySize > 0);

	data.pinGridBuffer = cvsArgs.pinGridBuffer;
	data.pinGridBuffer->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	data.pinGridBuffer->changeSize(pinGridMemorySize);
	bool bufferRecreated = data.pinGridBuffer->recreate();

	if (!cvsArgs.forceFullUpdate && !bufferRecreated && *args.pExecutionCounter % (args.config.subSampleMode * args.config.subSampleMode) != 0)
	{
		return data;
	}

	CVSUpdateMode updateMode = (bufferRecreated || cvsArgs.forceFullUpdate) ? CVSUpdateMode::ALL : cvsArgs.defaultUpdateMode;

	ComputeCmd    cmd;
	switch (updateMode)	
	{
		case CVSUpdateMode::ALL:
			cmd = ComputeCmd(pinGridCellCount, shaderPath + "pins/update_all.comp");
			break;
		case CVSUpdateMode::TRACE:
			cmd = ComputeCmd(cvsArgs.traceUpdateArgs.rayCount, shaderPath + "pins/update_trace.comp");
			break;
		default:
			DEBUG_BREAK;
			break;
	}
	Buffer scalarFieldUniform = default_scene::cmdGetScalarFieldUniform(cmdBuf, gState.frame->stack, args.sceneParams.densityScale, args.sceneParams.minDensity);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	cmd.pushSubmodule(cVkaShaderModulePath + "pt_scalar_field.glsl");
	default_scene::bindScalarField(cmd, args.resources.mediumTexture, args.config.rayMarchStepSize * cvsArgs.rayMarchingCoefficient, scalarFieldUniform);
	cmd.pushLocal();
	cmd.pushDescriptor(data.pinGridBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_POS_GRID_SIZE", cvsArgs.pinGridExtent.positionGridSize});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_DIR_GRID_SIZE", cvsArgs.pinGridExtent.directionGridSize});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_WRITE_STEP_SIZE", std::to_string(cvsArgs.traceUpdateArgs.writePinStepSize)});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_TYPE", static_cast<uint32_t>(cvsArgs.pinArgs.type)});
	struct PushStruct
	{
		uint32_t executionID;
	} pc;
	pc.executionID = *args.pExecutionCounter;
	cmd.pushConstant(&pc, sizeof(PushStruct));
	cmd.exec(cmdBuf);
	return data;
}

void bindCVSModule(ComputeCmd &cmd, const CVSData &data, const CVSArgs &args)
{
	cmd.pushSubmodule(shaderPath + "custom_volume_sampler.glsl");
	if (args.pinArgs.type != PinType::NONE)
	{
		cmd.pushDescriptor(data.pinGridBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	}
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_POS_GRID_SIZE", args.pinGridExtent.positionGridSize});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_DIR_GRID_SIZE", args.pinGridExtent.directionGridSize});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_TYPE", static_cast<uint32_t>(args.pinArgs.type)});
	cmd.pipelineDef.shaderDef.args.push_back({"SAMPLE_TYPE", static_cast<uint32_t>(args.pinArgs.sampleMode)});
	cmd.pipelineDef.shaderDef.args.push_back({"RNG_SAMPLE_MASK_ITERATIONS", args.pinArgs.bitMaskIterations});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_TYPE", static_cast<uint32_t>(args.pinArgs.type)});
	cmd.pushSpecializationConst(args.pinArgs.jitterPos);
	cmd.pushSpecializationConst(args.pinArgs.jitterDir);
}
