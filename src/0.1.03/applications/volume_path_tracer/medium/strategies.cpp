#include "strategies.h"
#include "../config.h"
#include "../shader_interface.h"
using namespace vka;


GVar gvar_perlin_scale0{"scale 0", 0.5f, GVAR_UNORM, PERLIN_NOISE_SETTINGS};
GVar gvar_perlin_scale1{"scale 1", 0.5f, GVAR_UNORM, PERLIN_NOISE_SETTINGS};
GVar gvar_perlin_frequency0{"frequency 0", 0.5f, GVAR_UNORM, PERLIN_NOISE_SETTINGS};
GVar gvar_perlin_frequency1{"frequency 1", 0.5f, GVAR_UNORM, PERLIN_NOISE_SETTINGS};
GVar gvar_perlin_falloff{"Falloff", false, GVAR_BOOL, PERLIN_NOISE_SETTINGS};
GVar gvar_medium_albedo_r{"Medium Albedo R", 1.0f, GVAR_UNORM, PERLIN_NOISE_SETTINGS};
GVar gvar_medium_albedo_g{"Medium Albedo G", 1.0f, GVAR_UNORM, PERLIN_NOISE_SETTINGS};
GVar gvar_medium_albedo_b{"Medium Albedo B", 1.0f, GVAR_UNORM, PERLIN_NOISE_SETTINGS};

bool PerlinVolume::requiresUpdate()
{
	// clang-format off
	return resolution != gvar_image_resolution.val.v_uint
		|| perlinFrequency0 != gvar_perlin_frequency0.val.v_float
		|| perlinScale0 != gvar_perlin_scale0.val.v_float
		|| perlinFrequency1 != gvar_perlin_frequency1.val.v_float
		|| perlinScale1 != gvar_perlin_scale1.val.v_float
		|| perlinFalloff != gvar_perlin_falloff.val.bool32();
	// clang-format on
}

void PerlinVolume::generateVolumeGrid(CmdBuffer cmdBuf, Image volume)
{
	resolution = gvar_image_resolution.val.v_uint;
	perlinFrequency0 = gvar_perlin_frequency0.val.v_float;
	perlinScale0 = gvar_perlin_scale0.val.v_float;
	perlinFrequency1 = gvar_perlin_frequency1.val.v_float;
	perlinScale1 = gvar_perlin_scale1.val.v_float;
	perlinFalloff = gvar_perlin_falloff.val.bool32();

	volume->changeExtent(VkExtent3D{resolution, resolution, resolution});
	volume->recreate();
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
	pc.resolution       = resolution;
	pc.perlinFalloff    = perlinFalloff;
	pc.perlinFrequency0 = perlinFrequency0;
	pc.perlinScale0     = perlinScale0;
	pc.perlinFrequency1 = perlinFrequency1;
	pc.perlinScale1     = perlinScale1;
	cmd.pushConstant(&pc, sizeof(PushStruct));

	cmd.pushDescriptor(volume, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	cmd.exec(cmdBuf);
}


