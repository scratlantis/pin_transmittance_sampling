#include "PipelineLayout.h"

vka::PipelineLayout::PipelineLayout(ResourceTracker *pTracker, const PipelineLayoutDefinition &definition) :
	UniqueResource(pTracker),
	definition(definition)
{
}

vka::PipelineLayout::~PipelineLayout()
{
}