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
			pinStructSize = sizeof(GLSLPinCacheEntryV1);
			break;
		case PinType::V3:
			pinStructSize = sizeof(GLSLPinCacheEntryV1);
			break;
	}

	uint32_t pinGridCellCount = cubed(cvsArgs.pinGridExtent.positionGridSize) * squared(cvsArgs.pinGridExtent.directionGridSize);
	uint32_t pinGridMemorySize = pinGridCellCount * pinStructSize;
	VKA_ASSERT(pinGridMemorySize > 0);

	gState.dataCache->fetch(data.pinGridBuffer, hasher("pin_buf"));
	data.pinGridBuffer->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	data.pinGridBuffer->changeSize(pinGridMemorySize);
	CVSUpdateMode updateMode = data.pinGridBuffer->recreate() ? CVSUpdateMode::ALL : cvsArgs.defaultUpdateMode;

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
	cmd.pushSubmodule(cVkaShaderModulePath + "pt_scalar_field.glsl");
	default_scene::bindScalarField(cmd, args.mediumTexture, args.rayMarchStepSize * cvsArgs.rayMarchingCoefficient);
	cmd.pushLocal();
	cmd.pushDescriptor(data.pinGridBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_POS_GRID_SIZE", cvsArgs.pinGridExtent.positionGridSize});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_DIR_GRID_SIZE", cvsArgs.pinGridExtent.directionGridSize});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_WRITE_STEP_SIZE", cvsArgs.traceUpdateArgs.writePinStepSize});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_TYPE", static_cast<uint32_t>(cvsArgs.pinArgs.type)});
	struct PushStruct
	{
		uint32_t executionID;
	} pc;
	pc.executionID = cvsArgs.frameID;
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
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_SAMPLE_MODE", static_cast<uint32_t>(args.pinArgs.sampleMode)});
	cmd.pipelineDef.shaderDef.args.push_back({"RNG_SAMPLE_MASK_ITERATIONS", args.pinArgs.bitMaskIterations});
	cmd.pushSpecializationConst(args.pinArgs.jitterPos);
	cmd.pushSpecializationConst(args.pinArgs.jitterDir);
}
