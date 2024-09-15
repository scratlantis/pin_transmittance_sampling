#include "pt_strategies.h"

GVar gvar_min_pin_bounce{"Min pin bounce", 1, GVAR_UINT_RANGE, PIN_SETTINGS, {0, 16}};
GVar gvar_max_bounce{"Max bounces", 8, GVAR_UINT_RANGE, PIN_SETTINGS, {1, 16}};
GVar gvar_raymarche_step_size{"Raymarche step size", 0.1f, GVAR_FLOAT_RANGE, PIN_SETTINGS, {0.05f, 0.3f}};

void ReferencePathTracer::trace(CmdBuffer cmdBuf, Image localTarget, const RenderInfo &renderInfo, Buffer lineSegmentBuffer)
{
	// Config general parameters
	ComputeCmd computeCmd = ComputeCmd(localTarget->getExtent2D(), shaderPath + "path_tracing/pt.comp");

	sConst.write(cmdBuf, computeCmd, localTarget->getExtent2D(), renderInfo.pCamera, renderInfo.frameIdx);

	bind_shader_const(computeCmd, sConst);

	bind_scene(computeCmd, renderInfo.pSceneData);

	bind_medium(computeCmd, renderInfo.pMediun);

	begin_local_descriptors(computeCmd);
	bind_target(computeCmd, localTarget);
	computeCmd.pushDescriptor(renderInfo.mediumInstanceBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	computeCmd.pushDescriptor(lineSegmentBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	set_general_params(computeCmd);

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	computeCmd.exec(cmdBuf);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
}



void PinPathTracer::trace(CmdBuffer cmdBuf, Image localTarget, const RenderInfo &renderInfo, Buffer lineSegmentBuffer)
{
	// Config general parameters
	ComputeCmd computeCmd = ComputeCmd(localTarget->getExtent2D(), shaderPath + "path_tracing/pt.comp",
	                                   {
	                                       {"USE_PINS", ""},
	                                   });

	sConst.write(cmdBuf, computeCmd, localTarget->getExtent2D(), renderInfo.pCamera, renderInfo.frameIdx);

	bind_shader_const(computeCmd, sConst);

	bind_scene(computeCmd, renderInfo.pSceneData);

	bind_medium(computeCmd, renderInfo.pMediun);

	begin_local_descriptors(computeCmd);
	bind_target(computeCmd, localTarget);
	computeCmd.pushDescriptor(renderInfo.mediumInstanceBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	computeCmd.pushDescriptor(lineSegmentBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	computeCmd.pipelineDef.shaderDef.args.push_back({"WRITE_LINE_SEGMENTS", ""});
	set_general_params(computeCmd);


	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	computeCmd.exec(cmdBuf);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

}
