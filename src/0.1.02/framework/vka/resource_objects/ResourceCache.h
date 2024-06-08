#pragma once
#include <unordered_map>
#include <vka/core/common.h>

#include <vka/resource_objects/DescriptorSetLayout.h>
#include <vka/resource_objects/PipelineLayout.h>
#include <vka/resource_objects/RasterizationPipeline.h>
#include <vka/resource_objects/ComputePipeline.h>
#include <vka/resource_objects/RenderPass.h>
#include <vka/resource_objects/Sampler.h>
#include <vka/resource_objects/Shader.h>
namespace vka
{


class ResourceCache
{
  private:
	std::unordered_map<DescriptorSetLayoutDefinition, DescriptorSetLayout>       descSetLayouts;
	std::unordered_map<PipelineLayoutDefinition, PipelineLayout>               pipelineLayouts;
	std::unordered_map<RasterizationPipelineDefinition, RasterizationPipeline> rasterizationPipelines;
	std::unordered_map<ComputePipelineDefinition, ComputePipeline>             computePipelines;
	std::unordered_map<RenderPassDefinition, RenderPass>                       renderPasses;
	std::unordered_map<SamplerDefinition, Sampler>                             samplers;
	std::unordered_map<ShaderDefinition, Shader>                             shaders;

  public:
	DescriptorSetLayout   *fetch(DescriptorSetLayoutDefinition const &rID);
	PipelineLayout        *fetch(PipelineLayoutDefinition const &rID);
	Shader                *fetch(ShaderDefinition const &rID);
	RenderPass            *fetch(RenderPassDefinition const &rID);
	RasterizationPipeline *fetch(RasterizationPipelineDefinition const &rID);
	ComputePipeline       *fetch(ComputePipelineDefinition const &rID);
	Sampler               *fetch(SamplerDefinition const &rID);

	VkDescriptorSetLayout fetchHandle(DescriptorSetLayoutDefinition const &rID);
	VkPipelineLayout      fetchHandle(PipelineLayoutDefinition const &rID);
	VkShaderModule        fetchHandle(ShaderDefinition const &rID);
	VkRenderPass          fetchHandle(RenderPassDefinition const &rID);
	VkPipeline            fetchHandle(RasterizationPipelineDefinition const &rID);
	VkPipeline            fetchHandle(ComputePipelineDefinition const &rID);
	VkSampler             fetchHandle(SamplerDefinition const &rID);

	void clear_descSetLayouts();
	void                  clear();
	ResourceCache(){};
	~ResourceCache(){};
	DELETE_COPY_CONSTRUCTORS(ResourceCache);
};
}        // namespace vka
