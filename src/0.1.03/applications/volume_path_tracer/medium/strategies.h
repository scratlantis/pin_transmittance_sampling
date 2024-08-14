#pragma once
#include "Medium.h"
class PerlinVolume : public VolumeGenerationStrategy
{
  public:
	Image generateVolumeGrid(CmdBuffer cmdBuf, uint32_t gridResolution) override;
};