#pragma once
#include "config.h"
#include "medium/Medium.h"
#include "shader_interface.h"
#include "TimeQueryManager.h"

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

	virtual void trace(CmdBuffer cmdBuf, Image localTarget, const RenderInfo &renderInfo, Buffer lineSegmentBuffer) = 0;
};

class ComparativePathTracer
{

  public:
	Image localTargetA;
	Image localTargetB;

	Image localAccumulationTargetA;
	Image localAccumulationTargetB;

	ShaderConst shaderConst{};

	PathTraceStrategy *pStrategieA;
	PathTraceStrategy *pStrategieB;


	MSEComputeResources mseRes;
	Buffer              mseBuffer;

	Buffer lineSegmentBuffer;
	Buffer lineSegmentInstanceBuffer;
	uint32_t lineSegmentCount;


	TimeQueryManager tqManager;
	bool timeQueryFinished = true;

	ComparativePathTracer() = default;
	~ComparativePathTracer() = default;

	void reset(CmdBuffer cmdBuf, PathTraceStrategy *pStrategieA, PathTraceStrategy *pStrategieB);
	void render(CmdBuffer cmdBuf, const RenderInfo &renderInfo);
	void showSplitView(CmdBuffer cmdBuf, Image target, float splittCoef, VkRect2D_OP targetArea);
	void showDiff(CmdBuffer cmdBuf, Image target, VkRect2D_OP targetArea);
	float computeMSE(CmdBuffer cmdBuf);
	ComparativePathTracer(float relativeWidth, float relativeHeight, Buffer lineSegmentInstanceBuffer, uint32_t lineSegmentCount);
	void destroy();

  private:
	ComputeCmd getCmdWriteLineSegments();
};
