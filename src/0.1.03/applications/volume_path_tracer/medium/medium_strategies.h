#pragma once
#include "Medium.h"
class PerlinVolume : public VolumeGenerationStrategy
{
  private:
	  uint32_t resolution;
	  float    perlinFrequency0;
	  float    perlinScale0;
	  float    perlinFrequency1;
	  float    perlinScale1;
	  uint32_t perlinFalloff;
  public:
	bool requiresUpdate() override;
	void generateVolumeGrid(CmdBuffer cmdBuf, Image volume) override;
};

class UniformPinGenerator : public PinDistributionStrategy
{
	uint32_t pinCount;
  public:
	bool requiresUpdate() override;
	void generatePins(CmdBuffer cmdBuf, Buffer pinBuffer) override;
};

class ArrayTransmittanceEncoder : public PinTransmittanceStorageStrategy
{
	uint32_t transmittanceValueCount;
  public:
	bool requiresUpdate() override;
	void computeTransmittance(CmdBuffer cmdBuf, BufferRef pinBuffer, Image volume, Buffer transmittanceBuffer, Buffer pinDensityMask) override;
};


class PinGridGenerator : public PinGridGenerationStrategy
{
	uint32_t gridSize;
	uint32_t pinCountPerGridCell;
  public:
	bool requiresUpdate() override;
	void generatePinGrid(CmdBuffer cmdBuf, BufferRef pinBuffer, BufferRef pinDensityMask, Buffer pinGridBuffer) override;
};