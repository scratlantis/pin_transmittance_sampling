#pragma once
#include "DescriptorSetLayout.h"
#include "PipelineLayout.h"
#include "Shader.h"
#include "RenderPass.h"
#include "Resource.h"
#include <vka/state_objects/global_state.h>

namespace vka
{
class RestarizationPipelineDefinition : public ResourceIdentifier
{
  public:
	

	hash_t hash() const;
	bool   _equals(ResourceIdentifier const &other) const override;
	bool   equals(RestarizationPipelineDefinition const &other) const;

  protected:
};

class RasterizationPipeline : public CachableResource
{
  private:
	VkPipeline handle;

  protected:
  public:
	virtual bool     _equals(Resource const &other) const override;
	virtual hash_t   hash() const override;
	virtual void     free() override;
	VkPipeline       getHandle() const;
	RasterizationPipeline(RestarizationPipelineDefinition const &definition);
};
}        // namespace vka