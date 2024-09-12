#include "pt_strategies.h"

GVar gvar_min_pin_bounce{"Min pin bounce", 1, GVAR_UINT_RANGE, PIN_SETTINGS, {0, 16}};
GVar gvar_max_bounce{"Max bounces", 8, GVAR_UINT_RANGE, PIN_SETTINGS, {1, 16}};
GVar gvar_raymarche_step_size{"Raymarche step size", 0.1f, GVAR_FLOAT_RANGE, PIN_SETTINGS, {0.05f, 0.3f}};

void ReferencePathTracer::trace(CmdBuffer cmdBuf, Image localTarget, const RenderInfo &renderInfo)
{
	// Config general parameters
	ComputeCmd computeCmd = ComputeCmd(localTarget->getExtent2D(), shaderPath + "path_tracing/pt.comp",
	                                   {
	                                       {"FORMAT1", getGLSLFormat(localTarget->getFormat())},
	                                       {"MAX_BOUNCES", gvar_max_bounce.val.v_uint},
										   {"AVERAGE_RAY_MARCHE_STEP_SIZE", std::to_string(gvar_raymarche_step_size.val.v_float)},
	                                   });

	sConst.write(cmdBuf, computeCmd, localTarget->getExtent2D(), renderInfo.pCamera, renderInfo.frameIdx);
	// Bind Constants
	bind_block_3(computeCmd, sConst);

	// Bind Target
	computeCmd.pushDescriptor(localTarget, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

	// Bind Scene
	bind_block_12(computeCmd, *renderInfo.pSceneData);
	add_shader_args(computeCmd, *renderInfo.pSceneData);
	// Bind Medium
	struct PushStruct
	{
		uint volRes;
		uint pinGridSize;
		uint pinCountPerGridCell;
		uint pinTransmittanceValueCount;
	} pc;
	pc.volRes                     = gvar_image_resolution.val.v_uint;
	pc.pinGridSize                = gvar_pin_grid_size.val.v_uint;
	pc.pinCountPerGridCell        = gvar_pin_count_per_grid_cell.val.v_uint;
	pc.pinTransmittanceValueCount = gvar_pin_transmittance_value_count.val.v_uint;
	computeCmd.pushConstant(&pc, sizeof(PushStruct));
	computeCmd.pushDescriptor(renderInfo.pMediun->volumeGrid, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	computeCmd.pushDescriptor(renderInfo.mediumInstanceBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	// Pins
	{
		computeCmd.pushDescriptor(renderInfo.pMediun->pinGrid, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		computeCmd.pushDescriptor(renderInfo.pMediun->pinTransmittance, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	}

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	computeCmd.exec(cmdBuf);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
}



void PinPathTracer::trace(CmdBuffer cmdBuf, Image localTarget, const RenderInfo &renderInfo)
{
	// Config general parameters
	ComputeCmd computeCmd = ComputeCmd(localTarget->getExtent2D(), shaderPath + "path_tracing/pt.comp",
	                                   {
	                                       {"FORMAT1", getGLSLFormat(localTarget->getFormat())},
	                                       {"USE_PINS", ""},
	                                       {"MAX_BOUNCES", gvar_max_bounce.val.v_uint},
	                                       {"MIN_PIN_BOUNCE", gvar_min_pin_bounce.val.v_uint},
	                                       {"PIN_MODE", "PIN_MODE_FLAG_DIRECT"},
	                                       {"PIN_COUNT_PER_GRID_CELL", gvar_pin_count_per_grid_cell.val.v_uint},
	                                       {"AVERAGE_RAY_MARCHE_STEP_SIZE",  std::to_string(gvar_raymarche_step_size.val.v_float)},
	                                   });

	sConst.write(cmdBuf, computeCmd, localTarget->getExtent2D(), renderInfo.pCamera, renderInfo.frameIdx);
	// Bind Constants
	bind_block_3(computeCmd, sConst);

	// Bind Target
	computeCmd.pushDescriptor(localTarget, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

	// Bind Scene
	bind_block_12(computeCmd, *renderInfo.pSceneData);
	add_shader_args(computeCmd, *renderInfo.pSceneData);
	// Bind Medium
	struct PushStruct
	{
		uint volRes;
		uint pinGridSize;
		uint pinCountPerGridCell;
		uint pinTransmittanceValueCount;
	} pc;
	pc.volRes                     = gvar_image_resolution.val.v_uint;
	pc.pinGridSize                = gvar_pin_grid_size.val.v_uint;
	pc.pinCountPerGridCell        = gvar_pin_count_per_grid_cell.val.v_uint;
	pc.pinTransmittanceValueCount = gvar_pin_transmittance_value_count.val.v_uint;
	computeCmd.pushConstant(&pc, sizeof(PushStruct));
	computeCmd.pushDescriptor(renderInfo.pMediun->volumeGrid, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	computeCmd.pushDescriptor(renderInfo.mediumInstanceBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	// Pins
	{
		computeCmd.pushDescriptor(renderInfo.pMediun->pinGrid, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		computeCmd.pushDescriptor(renderInfo.pMediun->pinTransmittance, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	}

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	computeCmd.exec(cmdBuf);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

}



void OldReferencePathTracer::trace(CmdBuffer cmdBuf, Image localTarget, const RenderInfo &renderInfo)
{
	// Config general parameters
	ComputeCmd computeCmd = ComputeCmd(localTarget->getExtent2D(), shaderPath + "path_tracing/pt.comp",
	                                   {
	                                       {"FORMAT1", getGLSLFormat(localTarget->getFormat())},
	                                       {"OLD_NO_DIRECT_ILLUM", ""},
	                                   });

	sConst.write(cmdBuf, computeCmd, localTarget->getExtent2D(), renderInfo.pCamera, renderInfo.frameIdx);
	// Bind Constants
	bind_block_3(computeCmd, sConst);

	// Bind Target
	computeCmd.pushDescriptor(localTarget, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

	// Bind Scene
	bind_block_12(computeCmd, *renderInfo.pSceneData);
	add_shader_args(computeCmd, *renderInfo.pSceneData);

	// Bind Medium
	struct PushStruct
	{
		uint volRes;
		uint pinGridSize;
		uint pinCountPerGridCell;
		uint pinTransmittanceValueCount;
	} pc;
	pc.volRes                     = gvar_image_resolution.val.v_uint;
	pc.pinGridSize                = gvar_pin_grid_size.val.v_uint;
	pc.pinCountPerGridCell        = gvar_pin_count_per_grid_cell.val.v_uint;
	pc.pinTransmittanceValueCount = gvar_pin_transmittance_value_count.val.v_uint;
	computeCmd.pushConstant(&pc, sizeof(PushStruct));
	computeCmd.pushDescriptor(renderInfo.pMediun->volumeGrid, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	computeCmd.pushDescriptor(renderInfo.mediumInstanceBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	// Pins
	{
		computeCmd.pushDescriptor(renderInfo.pMediun->pinGrid, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		computeCmd.pushDescriptor(renderInfo.pMediun->pinTransmittance, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	}

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	computeCmd.exec(cmdBuf);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
}
