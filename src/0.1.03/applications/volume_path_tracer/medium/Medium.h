#pragma once
#include <vka/vka.h>

class VolumeGenerationStrategy
{
  public:
	virtual Image generateVolumeGrid(CmdBuffer cmdBuf, uint32_t gridResolution) = 0;
};
class PinDistributionStrategy
{
  public:
	virtual Buffer generatePins(CmdBuffer cmdBuf, uint32_t pinCount) = 0;
};
class PinTransmittanceStorageStrategy
{
  public:
	virtual Buffer computeTransmittance(CmdBuffer cmdBuf, Buffer pinBuffer, uint32_t pinCount) = 0;
};
class PinGridGenerationStrategy
{
  public:
	virtual Buffer generatePinGrid(CmdBuffer cmdBuf, Buffer pinBuffer, uint32_t pinCount) = 0;
};



class Medium
{
  public:
	// Params
	uint32_t volumeGridResolution;
	uint32_t pinGridResolution;
	uint32_t numPins;
	uint32_t pinResolution;

	// Strategies
	VolumeGenerationStrategy *volGenerator;
	PinDistributionStrategy *pinGenerator;
	PinTransmittanceStorageStrategy *transmittanceEncoder;
	PinGridGenerationStrategy *pinGridGenerator;

	// Resources
	Image volumeGrid;
	Buffer pins;
	Buffer pinTransmittance;
	Buffer pinGrid;


	void build(CmdBuffer cmdBuf)
	{
		volumeGrid       = volGenerator->generateVolumeGrid(cmdBuf, volumeGridResolution);
		/*pins             = pinGenerator->generatePins(cmdBuf, numPins);
		pinTransmittance = transmittanceEncoder->computeTransmittance(cmdBuf, pins, numPins);
		pinGrid          = pinGridGenerator->generatePinGrid(cmdBuf, pins, numPins);*/
	}


	// compute volume grid
	// generate pins on gpu
	// compute transmittance per pin
	// compute pin grid
  private:

};