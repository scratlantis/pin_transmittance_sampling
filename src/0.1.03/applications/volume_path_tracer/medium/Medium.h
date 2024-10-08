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
	virtual void computeTransmittance(CmdBuffer cmdBuf, BufferRef pinBuffer, Image volume, Buffer transmittanceBuffer, Buffer pinDensityMask) = 0;
	/*virtual void computeTransmittance(CmdBuffer cmdBuf, BufferRef pinBuffer, Image volume, Buffer transmittanceBuffer) = 0;*/
};
class PinGridGenerationStrategy
{
  public:
	virtual bool requiresUpdate()                                                             = 0;
	virtual void generatePinGrid(CmdBuffer cmdBuf, BufferRef pinBuffer, BufferRef pinDensityMask, Buffer pinGridBuffer) = 0;
};

struct MediumBuildInfo
{
	VolumeGenerationStrategy        *volGenerator;
	PinDistributionStrategy         *pinGenerator;
	PinTransmittanceStorageStrategy *transmittanceEncoder;
	PinGridGenerationStrategy       *pinGridGenerator;

	
	bool isValid();
	MediumBuildTasks update(const MediumBuildInfo &other);
};

class Medium
{
	MediumBuildInfo currentBuildInfo;

  public:
	// Resources
	Image volumeGrid = nullptr;
	Buffer pins       = nullptr;
	Buffer pinTransmittance = nullptr;
	Buffer pinDensityMask = nullptr;
	Buffer pinGrid          = nullptr;


	

	MediumBuildTasks update(CmdBuffer cmdBuf, MediumBuildInfo buildInfo);
};

