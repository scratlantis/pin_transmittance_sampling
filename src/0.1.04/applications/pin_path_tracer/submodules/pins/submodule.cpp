#include "submodule.h"

namespace pins
{

void cmdUpdatePinGrid(CmdBuffer cmdBuf, Buffer pinGridBuf, Image scalarField, PinUpdateArgs args)
{
	uint32_t pinGridSize = cubed(args.posGridSize) * squared(args.dirGridSize)
		* (sizeof(float) + args.bitMaskSize * sizeof(uint32_t));
		//* sizeof(GLSLPinCacheEntry);
	pinGridBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	pinGridBuf->changeSize(pinGridSize);
	if (pinGridSize > 0 && pinGridBuf->recreate())
	{
		cmdFillBuffer<float>(cmdBuf, pinGridBuf, std::numeric_limits<float>::max());
		cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		           VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT);
		args.count == 1000000;
	}

	if (args.count == 0)
	{
		return;
	}

	ComputeCmd cmd;
	if (args.updateAll)
	{
		cmd = ComputeCmd(uvec3(args.posGridSize), shaderPath + "submodules/pins/trace_pins2.comp");
	}
	else
	{
		cmd = ComputeCmd(args.count, shaderPath + "submodules/pins/trace_pins.comp");
	}

	cmd.pushSubmodule(cVkaShaderModulePath + "pt_scalar_field.glsl");
	default_scene::bindScalarField(cmd, scalarField, args.rayMarchStepSize);
	cmd.pushLocal();
	cmd.pushDescriptor(pinGridBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushSpecializationConst(args.writePinStepSize);
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_POS_GRID_SIZE", args.posGridSize});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_DIR_GRID_SIZE", args.dirGridSize});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_MASK_SIZE", args.bitMaskSize});
	struct PushStruct
	{
		uint32_t executionID;
	} pc;
	pc.executionID = args.executionID;
	cmd.pushConstant(&pc, sizeof(PushStruct));
	cmd.exec(cmdBuf);
};

void cmdBindPins(ComputeCmd &cmd, Buffer pinGridBuf, PinSampleArgs args)
{
	cmd.pushSubmodule(shaderPath + "submodules/pins/pin_smd.glsl");
	cmd.pushDescriptor(pinGridBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushSpecializationConst(args.bitMaskIterations);
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_POS_GRID_SIZE", args.posGridSize});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_DIR_GRID_SIZE", args.dirGridSize});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_MASK_SIZE", args.bitMaskSize});
	if (args.disableBitmaskSampling)
	{
		cmd.pipelineDef.shaderDef.args.push_back({"DISABLE_BITMASK_SAMPLING", ""});
	}
};
}        // namespace pins