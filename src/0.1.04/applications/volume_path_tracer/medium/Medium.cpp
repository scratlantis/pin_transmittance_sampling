#include "Medium.h"
#include "config.h"

GVar gvar_medium_pos{"Medium Pos", {-0.2f,-0.2f,-0.2f}, GVAR_VEC3_RANGE, MEDIUM, {-4.0f, 4.0f}};
GVar gvar_medium_rot_y{"Medium Rotation Y Axis", 0.0f, GVAR_FLOAT_RANGE, MEDIUM, {0, 360.0f}};
GVar gvar_medium_scale{"Medium Scale", 0.3f, GVAR_FLOAT_RANGE, MEDIUM, {0.05f, 4.0f}};
GVar gvar_medium_albedo{"Medium Albedo", {1.f, 1.f, 1.f}, GVAR_VEC3_RANGE, MEDIUM, {0.f, 1.f}};


MediumBuildTasks Medium::update(CmdBuffer cmdBuf, MediumBuildInfo buildInfo)
{
	// Create resources if they don't exist
	if (!volumeGrid)
	{
		volumeGrid = createImage3D(gState.heap, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	}
	if (!pins)
	{
		pins = createBuffer(gState.heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	}
	if (!pinTransmittance)
	{
		pinTransmittance = createBuffer(gState.heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		pinDensityMask   = createBuffer(gState.heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	}
	if (!pinGrid)
	{
		pinGrid = createBuffer(gState.heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	}
	// Fetch tasks
	MediumBuildTasks tasks = currentBuildInfo.update(buildInfo);
	VKA_ASSERT(currentBuildInfo.isValid());

	// Execute tasks
	if (tasks.buildVolumeGrid)
	{
		currentBuildInfo.volGenerator->generateVolumeGrid(cmdBuf, volumeGrid);
	}

	if (tasks.buildPinBuffer)
	{
		currentBuildInfo.pinGenerator->generatePins(cmdBuf, pins);
		setDebugMarker(pins, "Pins");
	}
	if ((tasks.buildPinBuffer || tasks.buildVolumeGrid) && (tasks.buildTransmittanceBuffer || tasks.buildPinGrid))
	{
		cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	}

	if (tasks.buildTransmittanceBuffer)
	{
		currentBuildInfo.transmittanceEncoder->computeTransmittance(cmdBuf, pins, volumeGrid, pinTransmittance, pinDensityMask);
	}

	if (tasks.buildPinGrid)
	{
		currentBuildInfo.pinGridGenerator->generatePinGrid(cmdBuf, pins, pinDensityMask, pinGrid);
		setDebugMarker(pinGrid, "PinGrid");
	}

	return tasks;
}

bool MediumBuildInfo::isValid()
{
	return volGenerator && pinGenerator && transmittanceEncoder && pinGridGenerator;
};

MediumBuildTasks MediumBuildInfo::update(const MediumBuildInfo &other)
{
	MediumBuildTasks tasks{false, false, false, false};
	if (other.volGenerator)
	{
		tasks.buildVolumeGrid = volGenerator != other.volGenerator;
		volGenerator          = other.volGenerator;
	}
	if (other.pinGenerator)
	{
		tasks.buildPinBuffer = pinGenerator != other.pinGenerator;
		pinGenerator         = other.pinGenerator;
	}
	if (other.transmittanceEncoder)
	{
		tasks.buildTransmittanceBuffer = transmittanceEncoder != other.transmittanceEncoder;
		transmittanceEncoder           = other.transmittanceEncoder;
	}
	if (other.pinGridGenerator)
	{
		tasks.buildPinGrid = pinGridGenerator != other.pinGridGenerator;
		pinGridGenerator   = other.pinGridGenerator;
	}

	VKA_ASSERT(isValid());

	tasks.buildVolumeGrid          = tasks.buildVolumeGrid || volGenerator->requiresUpdate();
	tasks.buildPinBuffer           = tasks.buildPinBuffer || pinGenerator->requiresUpdate();
	tasks.buildTransmittanceBuffer = tasks.buildTransmittanceBuffer || tasks.buildVolumeGrid || tasks.buildPinBuffer || transmittanceEncoder->requiresUpdate();
	tasks.buildPinGrid             = tasks.buildPinGrid || tasks.buildPinBuffer || pinGridGenerator->requiresUpdate();
	return tasks;
}