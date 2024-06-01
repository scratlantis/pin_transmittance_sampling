#include "ComputePipeline.h"

vka::ComputePipeline::ComputePipeline(ResourceTracker *pTracker, const ComputePipelineState pipelineState) :
	UniqueResource(pTracker), pipelineState(pipelineState)
{
}

vka::ComputePipeline::~ComputePipeline()
{
}