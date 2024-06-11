#pragma once
#include "Resource.h"
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
	PipelineLayoutDefinition pipelineLayoutDefinition;
	std::vector<uint32_t> specialisationEntrySizes;
	std::vector<uint8_t>  specializationData;

	bool   operator==(const ResourceIdentifier &other) const override;
	bool   operator==(const ComputePipelineDefinition &other) const;
	hash_t hash() const override;
};

class ComputePipeline : public Cachable_T<VkPipeline>
{
  public:
	virtual void     free();
	ComputePipeline(ComputePipelineDefinition const &definition);
};
}		// namespace vka
DECLARE_HASH(vka::ComputePipelineDefinition, hash)