#pragma once
#include <vulkan/vulkan.h>
namespace vka
{
class DescriptorSetLayout;
class DescriptorSetLayoutDefinition;
class PipelineLayout;
class PipelineLayoutDefinition;
class RasterizationPipeline;
class RasterizationPipelineDefinition;
class ComputePipeline;
class ComputePipelineDefinition;
class RenderPass;
class RenderPassDefinition;
class Sampler;
class SamplerDefinition;
class Shader;
class ShaderDefinition;

class IResourceCache
{
  public:
	virtual VkDescriptorSetLayout fetch(DescriptorSetLayoutDefinition const &rID)   = 0;
	virtual VkPipelineLayout      fetch(PipelineLayoutDefinition const &rID)        = 0;
	virtual VkShaderModule        fetch(ShaderDefinition const &rID)                = 0;
	virtual VkRenderPass          fetch(RenderPassDefinition const &rID)            = 0;
	virtual VkPipeline            fetch(RasterizationPipelineDefinition const &rID) = 0;
	virtual VkPipeline            fetch(ComputePipelineDefinition const &rID)       = 0;
	virtual VkSampler             fetch(SamplerDefinition const &rID)               = 0;
	virtual void                  clearDescSetLayouts()                             = 0;
	virtual void                  clearPipelineLayouts()                            = 0;
	virtual void                  clearRasterizationPipelines()                     = 0;
	virtual void                  clearComputePipelines()                           = 0;
	virtual void                  clearRenderPasses()                               = 0;
	virtual void                  clearSamplers()                                   = 0;
	virtual void                  clearShaders()                                    = 0;
	virtual void                  clearAll()                                        = 0;
};
}        // namespace vka
