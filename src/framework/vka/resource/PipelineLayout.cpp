#include "PipelineLayout.h"

vka::PipelineLayout::PipelineLayout(const PipelineLayoutDefinition &definition, ResourceTracker *pTracker):
	UniqueResource(pTracker),
	definition(definition)
{
}

vka::PipelineLayout::~PipelineLayout()
{
}