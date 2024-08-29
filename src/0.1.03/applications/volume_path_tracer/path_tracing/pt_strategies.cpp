#include "pt_strategies.h"

void ReferencePathTracer::trace(CmdBuffer cmdBuf, Image localTarget, const RenderInfo &renderInfo)
{
	// Config general parameters
	ComputeCmd computeCmd = ComputeCmd(localTarget->getExtent2D(), shaderPath + "path_tracing/pt.comp",
	                                   {
	                                       {"FORMAT1", getGLSLFormat(localTarget->getFormat())},
	                                   });

	sConst.write(cmdBuf, computeCmd, localTarget->getExtent2D(), renderInfo.pCamera, renderInfo.frameIdx);
	// Bind Constants
	bind_block_3(computeCmd, sConst);

	// Bind Target
	computeCmd.pushDescriptor(localTarget, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

	// Bind Scene
	bind_block_10(computeCmd, *renderInfo.pSceneData);

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

	//cmdFill(cmdBuf, localTarget, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(1.0));
}

void PinPathTracer::trace(CmdBuffer cmdBuf, Image localTarget, const RenderInfo &renderInfo)
{
	// Config general parameters
	ComputeCmd computeCmd = ComputeCmd(localTarget->getExtent2D(), shaderPath + "path_tracing/pt.comp",
	                                   {
	                                       {"FORMAT1", getGLSLFormat(localTarget->getFormat())},
	                                       {"USE_PINS", ""},
	                                   });

	sConst.write(cmdBuf, computeCmd, localTarget->getExtent2D(), renderInfo.pCamera, renderInfo.frameIdx);
	// Bind Constants
	bind_block_3(computeCmd, sConst);

	// Bind Target
	computeCmd.pushDescriptor(localTarget, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

	// Bind Scene
	bind_block_10(computeCmd, *renderInfo.pSceneData);

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
