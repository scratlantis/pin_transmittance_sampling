#pragma once
#include <vka/vka.h>

struct MediumBuildTasks
{
	bool buildVolumeGrid;
	bool buildPinBuffer;
	bool buildTransmittanceBuffer;
	bool buildPinGrid;
};

class VolumeGenerationStrategy
{
  public:
	virtual bool requiresUpdate()                                   = 0;
	virtual void generateVolumeGrid(CmdBuffer cmdBuf, Image volume) = 0;
};
class PinDistributionStrategy
{
  public:
	virtual bool requiresUpdate()                                 = 0;
	virtual void generatePins(CmdBuffer cmdBuf, Buffer pinBuffer) = 0;
};
class PinTransmittanceStorageStrategy
{
  public:
	virtual bool requiresUpdate()                                                                        = 0;
	virtual void computeTransmittance(CmdBuffer cmdBuf, BufferRef pinBuffer, Image volume, Buffer transmittanceBuffer) = 0;
	/*virtual void computeTransmittance(CmdBuffer cmdBuf, BufferRef pinBuffer, Image volume, Buffer transmittanceBuffer) = 0;*/
};
class PinGridGenerationStrategy
{
  public:
	virtual bool requiresUpdate()                                                             = 0;
	virtual void generatePinGrid(CmdBuffer cmdBuf, BufferRef pinBuffer, Buffer pinGridBuffer) = 0;
};

struct MediumBuildInfo
{
	VolumeGenerationStrategy        *volGenerator;
	PinDistributionStrategy         *pinGenerator;
	PinTransmittanceStorageStrategy *transmittanceEncoder;
	PinGridGenerationStrategy       *pinGridGenerator;

	bool isValid()
	{
		return volGenerator && pinGenerator && transmittanceEncoder && pinGridGenerator;
	};

	MediumBuildTasks update(const MediumBuildInfo &other)
	{
		MediumBuildTasks tasks{false,false,false,false};
		if (other.volGenerator)
		{
			tasks.buildVolumeGrid = volGenerator != other.volGenerator;
			volGenerator = other.volGenerator;
		}
		if (other.pinGenerator)
		{
			tasks.buildPinBuffer = pinGenerator != other.pinGenerator;
			pinGenerator = other.pinGenerator;
		}
		if (other.transmittanceEncoder)
		{
			tasks.buildTransmittanceBuffer = transmittanceEncoder != other.transmittanceEncoder;
			transmittanceEncoder           = other.transmittanceEncoder;
		}
		if ( other.pinGridGenerator)
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
};

class Medium
{
	MediumBuildInfo currentBuildInfo;

  public:
	// Resources
	Image volumeGrid = nullptr;
	Buffer pins       = nullptr;
	Buffer pinTransmittance = nullptr;
	Buffer pinGrid          = nullptr;


	MediumBuildTasks update(CmdBuffer cmdBuf, MediumBuildInfo buildInfo)
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
			currentBuildInfo.transmittanceEncoder->computeTransmittance(cmdBuf, pins, volumeGrid, pinTransmittance);
		}

		if (tasks.buildPinGrid)
		{
			currentBuildInfo.pinGridGenerator->generatePinGrid(cmdBuf, pins, pinGrid);
			setDebugMarker(pinGrid, "PinGrid");
		}

		return tasks;
	}
  private:

};

