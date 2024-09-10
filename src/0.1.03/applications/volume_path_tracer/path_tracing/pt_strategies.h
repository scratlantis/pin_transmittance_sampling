#pragma once
#include "config.h"
#include "medium/Medium.h"
#include "ComparativePathTracer.h"


class OldReferencePathTracer : public PathTraceStrategy
{
  public:
	OldReferencePathTracer() = default;
	~OldReferencePathTracer() = default;

	void trace(CmdBuffer cmdBuf, Image localTarget, const RenderInfo &renderInfo) override;

  private:
};

class ReferencePathTracer : public PathTraceStrategy
{
  public:
	ReferencePathTracer() = default;
	~ReferencePathTracer() = default;


	void trace(CmdBuffer cmdBuf, Image localTarget, const RenderInfo &renderInfo) override;

  private:
};

class PinPathTracer : public PathTraceStrategy
{
  public:
	PinPathTracer() = default;
	~PinPathTracer() = default;

	void trace(CmdBuffer cmdBuf, Image localTarget, const RenderInfo &renderInfo) override;

  private:
};
