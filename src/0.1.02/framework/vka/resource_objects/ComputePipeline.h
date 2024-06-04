#pragma once
#include "Resource.h"
#include <vka/state_objects/global_state.h>
#include "PipelineLayout.h"
#include "Shader.h"

namespace vka
{
struct ComputePipelineDefinition : public ResourceIdentifier
{
  public:
	ComputePipelineDefinition(){};
	std::vector<VkSpecializationMapEntry> specMapEntries;
	VkSpecializationInfo                  specInfo;

	ShaderDefinition         shaderDef;
	PipelineLayoutDefinition pipelineLayoutDef;
	std::vector<uint32_t> specialisationEntrySizes;
	std::vector<uint8_t>  specializationData;

	hash_t hash() const override;
	bool _equals(ResourceIdentifier const &other) const override;
	bool   equals(ComputePipelineDefinition const &other) const;
};

class ComputePipeline : public CachableResource
{
  private:
	VkPipeline handle;
  public:
	virtual bool _equals(Resource const &other) const;
	virtual hash_t   hash() const;
	virtual void     free();
	VkPipeline       getHandle() const;
	ComputePipeline(ComputePipelineDefinition const &definition);
};
}		// namespace vka