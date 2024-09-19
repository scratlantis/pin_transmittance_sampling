#pragma once
#include "config.h"



class EventManager
{
  public:
	EventManager() = default;
	~EventManager() = default;

	void init(FixedCamera *pCam);
	void reset();
	void newFrame();

	bool requestModelLoad();

	void updateView();

	void updatePathTraceParams();

	FixedCamera *pCam;

	uint32_t       frameCounter    = 0;

	// View
	bool           viewHasChanged  = true;
	bool           debugView       = false;
	uint32_t       viewType        = 0;
	const uint32_t viewTypeCount   = 2;

	// Path Tracing
	float ptSplittCoef = 0.5f;
	bool ptReset = true;
	bool pathViewCreated = false;
	uint32_t lastConfig = _UI32_MAX;

	std::vector<GVar> gVarsLastFrame;
	std::vector<GVar> gVarsPreGui;
	bool guiFocus = false;
  private:
};