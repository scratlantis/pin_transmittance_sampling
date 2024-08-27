#pragma once
#include "config.h"
#include "medium/Medium.h"
#include "shader_interface.h"

struct RenderInfo
{
	uint32_t    frameIdx;
	Camera     *pCamera;
	Medium     *pMediun;
	Buffer      mediumInstanceBuffer;
	USceneData *pSceneData;
};

class PathTraceStrategy
{
  public:
	PathTraceStrategy()  = default;
	~PathTraceStrategy() = default;

	virtual void trace(CmdBuffer cmdBuf, Image localTarget, const RenderInfo &renderInfo) = 0;
};

class ComparativePathTracer
{

	Image localTargetA;
	Image localTargetB;

	Image localAccumulationTargetA;
	Image localAccumulationTargetB;

	ShaderConst shaderConst{};


  public:
	ComparativePathTracer() = default;
	~ComparativePathTracer() = default;

	void clearAccumulationTargets(CmdBuffer cmdBuf);
	void renderSplitView(CmdBuffer cmdBuf, PathTraceStrategy *pStrategieA, PathTraceStrategy *pStrategieB, Image target,
	                     float splittCoef, VkRect2D_OP targetArea, const RenderInfo &renderInfo);
	ComparativePathTracer(float relativeWidth, float relativeHeight);
};
