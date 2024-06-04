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
class RenderpassDefinition;
class Renderpass;
class SamplerDefinition;
class Sampler;

class ResourceCache
{
  private:
	std::map<DescriptorSetLayoutDefinition, DescriptorSetLayout>       descSetLayouts;
	std::map<PipelineLayoutDefinition, PipelineLayout>               pipelineLayouts;
	std::map<RasterizationPipelineDefinition, RasterizationPipeline> rasterizationPipelines;
	std::map<ComputePipelineDefinition, ComputePipeline>             computePipelines;
	std::map<RenderpassDefinition, Renderpass>                       renderpasses;
	std::map<SamplerDefinition, Sampler>                             samplers;

  public:
	DescriptorSetLayout*    fetch(DescriptorSetLayoutDefinition const &rID);
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
