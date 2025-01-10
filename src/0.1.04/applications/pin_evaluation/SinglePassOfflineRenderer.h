#pragma once
#include "config.h"
#include "pt_interface.h"
#include "OfflineRenderer.h"

class SIOfflineRenderer : public OfflineRenderer
{
  public:
	bool SIOfflineRenderer::cmdRunTick(CmdBuffer cmdBuf);
};
