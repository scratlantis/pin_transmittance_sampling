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