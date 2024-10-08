#include "../config.h"
#include "../shader_interface.h"
#include "medium_strategies.h"
using namespace vka;


GVar gvar_perlin_scale0{"scale 0", 0.5f, GVAR_UNORM, NOISE_SETTINGS};
GVar gvar_perlin_scale1{"scale 1", 0.5f, GVAR_UNORM, NOISE_SETTINGS};
GVar gvar_perlin_frequency0{"frequency 0", 0.5f, GVAR_UNORM, NOISE_SETTINGS};
GVar gvar_perlin_frequency1{"frequency 1", 0.5f, GVAR_UNORM, NOISE_SETTINGS};
GVar gvar_perlin_falloff{"Falloff", false, GVAR_BOOL, NOISE_SETTINGS};


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

GVar gvar_pin_count{"Pin count", 10000, GVAR_UINT_RANGE, PIN_SETTINGS, {1000, 100000}};

bool UniformPinGenerator::requiresUpdate()
{
	return gvar_pin_count.val.v_uint != pinCount;
}

void UniformPinGenerator::generatePins(CmdBuffer cmdBuf, Buffer pinBuffer)
{
	pinCount = gvar_pin_count.val.v_uint;
	pinBuffer->changeSize(pinCount * sizeof(GLSLPin));
	pinBuffer->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);

	pinBuffer->addUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	pinBuffer->recreate();

	ComputeCmd cmd = ComputeCmd(pinCount, shaderPath + "medium/gen_uniform_pins.comp");
	cmd.pushDescriptor(pinBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushConstant(&pinCount, sizeof(uint32_t));
	cmd.exec(cmdBuf);
}

GVar gvar_pin_transmittance_value_count{"Transmittance Value Count", 10, GVAR_UINT_RANGE, PIN_SETTINGS, {1, 20}};

bool ArrayTransmittanceEncoder::requiresUpdate()
{
	return transmittanceValueCount != gvar_pin_transmittance_value_count.val.v_uint;
}

void ArrayTransmittanceEncoder::computeTransmittance(CmdBuffer cmdBuf, BufferRef pinBuffer, Image volume, Buffer transmittanceBuffer, Buffer pinDensityMask)
{
	transmittanceValueCount = gvar_pin_transmittance_value_count.val.v_uint;
	transmittanceBuffer->changeSize(gvar_pin_count.val.v_uint * gvar_pin_transmittance_value_count.val.v_uint * sizeof(float));
	transmittanceBuffer->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
	transmittanceBuffer->recreate();

	pinDensityMask->changeSize(gvar_pin_count.val.v_uint * sizeof(GLSLDensityMaskEntry));
	pinDensityMask->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
	pinDensityMask->recreate();

	ComputeCmd cmd = ComputeCmd(gvar_pin_count.val.v_uint, shaderPath + "medium/compute_transmittance_array.comp");
	cmd.pushDescriptor(pinBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(transmittanceBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pinDensityMask, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(volume, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	struct PushStruct
	{
		uint32_t pinCount;
		uint32_t perPinValueCount;
	} pc;
	pc.pinCount = gvar_pin_count.val.v_uint;
	pc.perPinValueCount = transmittanceValueCount;
	cmd.pushConstant(&pc, sizeof(PushStruct));
	cmd.exec(cmdBuf);
}

GVar gvar_pin_count_per_grid_cell{"Pins per grid cell", 64, GVAR_UINT_RANGE, PIN_SETTINGS, {1, 128}};
GVar gvar_pin_grid_size{"Pin grid size", 16, GVAR_UINT_RANGE, PIN_SETTINGS, {4, 32}};

bool PinGridGenerator::requiresUpdate()
{
	return gridSize != gvar_pin_grid_size.val.v_uint || pinCountPerGridCell != gvar_pin_count_per_grid_cell.val.v_uint;
}

void PinGridGenerator::generatePinGrid(CmdBuffer cmdBuf, BufferRef pinBuffer, BufferRef pinDensityMask, Buffer pinGridBuffer)
{
	gridSize = gvar_pin_grid_size.val.v_uint;
	pinCountPerGridCell = gvar_pin_count_per_grid_cell.val.v_uint;
	pinGridBuffer->changeSize(gridSize * gridSize * gridSize * pinCountPerGridCell * sizeof(GLSLPinGridEntry));
	pinGridBuffer->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
	pinGridBuffer->recreate();
	ComputeCmd cmd = ComputeCmd(glm::uvec3(gridSize), shaderPath + "medium/fill_pin_grid_greedy.comp");
	cmd.pushDescriptor(pinBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pinDensityMask, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pinGridBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	struct PushStruct
	{
		uint32_t pinCount;
		uint32_t pinCountPerGridCell;
		uint32_t gridSize;
	} pc;
	pc.pinCount            = gvar_pin_count.val.v_uint;
	pc.pinCountPerGridCell = pinCountPerGridCell;
	pc.gridSize            = gridSize;
	cmd.pushConstant(&pc, sizeof(PushStruct));
	cmd.exec(cmdBuf);
}


