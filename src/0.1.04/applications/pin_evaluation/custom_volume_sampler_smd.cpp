#include "custom_volume_sampler_smd.h"
#include "shaders/pins/interface_structs.glsl"
#include "pt_interface.h"

extern GVar gvar_show_pin_grid_size;
extern GVar gvar_show_pin_grid_size_coef;


extern GVar gvar_pin_use_relative_size;
extern GVar gvar_pin_relative_size;





CVSData cmdPrepareCVSData(CmdBuffer cmdBuf, TraceArgs args)
{
	CVSArgs cvsArgs = args.cvsArgs;
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
	uint32_t pinGridCellCount;
	if (gvar_pin_use_relative_size.val.v_bool)
	{
		uint32_t textureSize = args.resources.mediumTexture->getMemorySize();
		uint32_t relativeSize = (uint32_t) textureSize * gvar_pin_relative_size.val.v_float;
		uint32_t gridCellCount = relativeSize / pinStructSize;
		uint32_t angularGridSize = squared(cvsArgs.pinGridExtent.directionGridSize) * 4;
		uint32_t positionGridSize = gridCellCount / angularGridSize;
		cvsArgs.pinGridExtent.positionGridSize = (uint32_t) std::pow((float) positionGridSize, 1.f / 3.f);
	}
	pinGridCellCount = cubed(cvsArgs.pinGridExtent.positionGridSize) * squared(cvsArgs.pinGridExtent.directionGridSize) * 4;


	uint32_t pinGridMemorySize = pinGridCellCount * pinStructSize;
	gvar_show_pin_grid_size.val.v_float      = pinGridMemorySize / (1024.0f * 1024.0f);
	gvar_show_pin_grid_size_coef.val.v_float = pinGridMemorySize / (1.0f * args.resources.mediumTexture->getMemorySize());

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
	Buffer scalarFieldUniform = default_scene::cmdGetScalarFieldUniform(cmdBuf, gState.frame->stack, args.sceneParams.densityScale, args.sceneParams.minDensity, args.sceneParams.maxDensity);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	cmd.pushSubmodule(cVkaShaderModulePath + "pt_scalar_field.glsl");
	default_scene::bindScalarField(cmd, args.resources.mediumTexture, cvsArgs.rayMarchingCoefficient, scalarFieldUniform);
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

void bindCVSModule(ComputeCmd &cmd, const CVSData &data, TraceArgs args)
{
	CVSArgs cvsArgs = args.cvsArgs;
	cmd.pushSubmodule(shaderPath + "custom_volume_sampler.glsl");
	if (cvsArgs.pinArgs.type != PinType::NONE)
	{
		cmd.pushDescriptor(data.pinGridBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
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
	if (pinStructSize > 0 && gvar_pin_use_relative_size.val.v_bool)
	{
		uint32_t textureSize                   = args.resources.mediumTexture->getMemorySize();
		uint32_t relativeSize                  = (uint32_t) textureSize * gvar_pin_relative_size.val.v_float;
		uint32_t gridCellCount                 = relativeSize / pinStructSize;
		uint32_t angularGridSize               = squared(cvsArgs.pinGridExtent.directionGridSize) * 4;
		uint32_t positionGridSize              = gridCellCount / angularGridSize;
		cvsArgs.pinGridExtent.positionGridSize = (uint32_t) std::pow((float) positionGridSize, 1.f / 3.f);
	}

	cmd.pipelineDef.shaderDef.args.push_back({"PIN_POS_GRID_SIZE", cvsArgs.pinGridExtent.positionGridSize});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_DIR_GRID_SIZE", cvsArgs.pinGridExtent.directionGridSize});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_TYPE", static_cast<uint32_t>(cvsArgs.pinArgs.type)});
	cmd.pipelineDef.shaderDef.args.push_back({"SAMPLE_TYPE", static_cast<uint32_t>(cvsArgs.pinArgs.sampleMode)});
	cmd.pipelineDef.shaderDef.args.push_back({"RNG_SAMPLE_MASK_ITERATIONS", cvsArgs.pinArgs.bitMaskIterations});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_TYPE", static_cast<uint32_t>(cvsArgs.pinArgs.type)});
	cmd.pushSpecializationConst(cvsArgs.pinArgs.jitterPos);
	cmd.pushSpecializationConst(cvsArgs.pinArgs.jitterDir);
}
