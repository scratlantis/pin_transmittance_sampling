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
	virtual DescriptorSetLayout   *fetch(DescriptorSetLayoutDefinition const &rID) = 0;
	virtual PipelineLayout        *fetch(PipelineLayoutDefinition const &rID) = 0;
	virtual Shader                *fetch(ShaderDefinition const &rID) = 0;
	virtual RenderPass            *fetch(RenderPassDefinition const &rID) = 0;
	virtual RasterizationPipeline *fetch(RasterizationPipelineDefinition const &rID) = 0;
	virtual ComputePipeline       *fetch(ComputePipelineDefinition const &rID) = 0;
	virtual Sampler               *fetch(SamplerDefinition const &rID) = 0;

	virtual VkDescriptorSetLayout fetchHandle(DescriptorSetLayoutDefinition const &rID) = 0;
	virtual VkPipelineLayout      fetchHandle(PipelineLayoutDefinition const &rID) = 0;
	virtual VkShaderModule        fetchHandle(ShaderDefinition const &rID) = 0;
	virtual VkRenderPass          fetchHandle(RenderPassDefinition const &rID) = 0;
	virtual VkPipeline            fetchHandle(RasterizationPipelineDefinition const &rID) = 0;
	virtual VkPipeline            fetchHandle(ComputePipelineDefinition const &rID) = 0;
	virtual VkSampler             fetchHandle(SamplerDefinition const &rID) = 0;
	virtual void                  clear();
};
}        // namespace vka
