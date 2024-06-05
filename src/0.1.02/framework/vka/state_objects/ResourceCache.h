#pragma once
#include <map>
#include <vka/core/common.h>
namespace vka
{
class Resource;
class ResourceIdentifier;

class DescriptorSetLayoutDefinition;
class DescriptorSetLayout;
class PipelineLayoutDefinition;
class PipelineLayout;
class RasterizationPipelineDefinition;
class RasterizationPipeline;
class ComputePipelineDefinition;
class ComputePipeline;
class RenderPassDefinition;
class RenderPass;
class SamplerDefinition;
class Sampler;
class ShaderDefinition;
class Shader;

class ResourceCache
{
  private:
	std::map<DescriptorSetLayoutDefinition, DescriptorSetLayout>       descSetLayouts;
	std::map<PipelineLayoutDefinition, PipelineLayout>               pipelineLayouts;
	std::map<RasterizationPipelineDefinition, RasterizationPipeline> rasterizationPipelines;
	std::map<ComputePipelineDefinition, ComputePipeline>             computePipelines;
	std::map<RenderPassDefinition, RenderPass>                       renderpasses;
	std::map<SamplerDefinition, Sampler>                             samplers;
	std::map<ShaderDefinition, Shader>                             shaders;

  public:
	DescriptorSetLayout*    fetch(DescriptorSetLayoutDefinition const &rID);
	PipelineLayout      *fetch(PipelineLayoutDefinition const &rID);
	Shader              *fetch(ShaderDefinition const &rID);
	RenderPass             *fetch(RenderPassDefinition const &rID);
	//PipelineLayout        fetch(PipelineLayoutDefinition const &rID);
	//RasterizationPipeline fetch(RasterizationPipelineDefinition const &rID);
	//ComputePipeline       fetch(ComputePipelineDefinition const &rID);
	//Renderpass            fetch(RenderpassDefinition const &rID);
	//Sampler               fetch(SamplerDefinition const &rID);
	void                  clear();
	ResourceCache(){};
	~ResourceCache(){};
	DELETE_COPY_CONSTRUCTORS(ResourceCache);
};
}        // namespace vka
