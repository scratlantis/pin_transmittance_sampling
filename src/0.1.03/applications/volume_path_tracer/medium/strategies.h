#pragma once
#include "Medium.h"
class PerlinVolume : public VolumeGenerationStrategy
{
  public:
	Image generateVolumeGrid(CmdBuffer cmdBuf, Image volume) override;
};