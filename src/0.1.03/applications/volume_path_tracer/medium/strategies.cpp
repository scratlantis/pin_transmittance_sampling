#include "strategies.h"
#include "../config.h"
#include "../shader_interface.h"
using namespace vka;

Image PerlinVolume::generateVolumeGrid(CmdBuffer cmdBuf, Image volume)
{
	cmdTransitionLayout(cmdBuf, volume, VK_IMAGE_LAYOUT_GENERAL);
	ComputeCmd cmd = ComputeCmd(volume->getExtent(), shaderPath + "medium/perlin_volume.comp");
	struct PushStruct
	{
		uint   resolution;
		float perlinFrequency0;
		float perlinScale0;
		float perlinFrequency1;
		float perlinScale1;
		uint   perlinFalloff;
	} pc;
	VKA_ASSERT(volume->getExtent().width == volume->getExtent().height && volume->getExtent().width == volume->getExtent().depth);
	pc.resolution       = volume->getExtent().width;
	pc.perlinFalloff	= gvar_perlin_falloff.val.v_uint;
	pc.perlinFrequency0 = gvar_perlin_frequency0.val.v_float;
	pc.perlinScale0     = gvar_perlin_scale0.val.v_float;
	pc.perlinFrequency1 = gvar_perlin_frequency1.val.v_float;
	pc.perlinScale1     = gvar_perlin_scale1.val.v_float;
	cmd.pushConstant(&pc, sizeof(PushStruct));
	cmd.pushDescriptor(volume, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	cmd.exec(cmdBuf);
	return volume;
}
