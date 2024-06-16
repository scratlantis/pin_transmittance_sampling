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


class ResourceCache : public IResourceCache
{
  private:
	std::unordered_map<DescriptorSetLayoutDefinition, DescriptorSetLayout>     descSetLayouts;
	std::unordered_map<PipelineLayoutDefinition, PipelineLayout>               pipelineLayouts;
	std::unordered_map<RasterizationPipelineDefinition, RasterizationPipeline> rasterizationPipelines;
	std::unordered_map<ComputePipelineDefinition, ComputePipeline>             computePipelines;
	std::unordered_map<RenderPassDefinition, RenderPass>                       renderPasses;
	std::unordered_map<SamplerDefinition, Sampler>                             samplers;
	std::unordered_map<ShaderDefinition, Shader>                               shaders;

	template <typename Def, typename Obj, typename Handle>
	static void fetch(std::unordered_map<Def, Obj> &map, Def const &rID, Handle &handle)
	{
		auto it = map.find(rID);
		if (it != map.end())
			handle = it->second.getHandle();
		else
		{
			Obj obj = Obj(gState.cache, rID); // civ
			map.insert({rID, obj});
			handle = obj.getHandle();
		}
	}
	template <typename Def, typename Obj>
	static void clear(std::unordered_map<Def, Obj> &map)
	{
	    auto it = map.begin();
	    for (auto it = map.begin(); it != map.end(); ++it)
	    {
	        it->second.free();
	    }
	    map.clear();
	}
  public:

	VkDescriptorSetLayout fetch(DescriptorSetLayoutDefinition const &rID) override
	{
		VkDescriptorSetLayout handle;
		fetch(descSetLayouts, rID, handle);
		return handle;
	}
	VkPipelineLayout fetch(PipelineLayoutDefinition const &rID) override
	{
		VkPipelineLayout handle;
		fetch(pipelineLayouts, rID, handle);
		return handle;
	}
	VkShaderModule fetch(ShaderDefinition const &rID) override
	{
		VkShaderModule handle;
		fetch(shaders, rID, handle);
		return handle;
	}
	VkRenderPass fetch(RenderPassDefinition const &rID) override
	{
		VkRenderPass handle;
		fetch(renderPasses, rID, handle);
		return handle;
	}
	VkPipeline fetch(RasterizationPipelineDefinition const &rID) override
	{
		VkPipeline handle;
		fetch(rasterizationPipelines, rID, handle);
		return handle;
	}
	VkPipeline fetch(ComputePipelineDefinition const &rID) override
	{
		VkPipeline handle;
		fetch(computePipelines, rID, handle);
		return handle;
	}
	VkSampler fetch(SamplerDefinition const &rID) override
	{
		VkSampler handle;
		fetch(samplers, rID, handle);
		return handle;
	}

	void clearDescSetLayouts() override
	{
		clear(descSetLayouts);

		// Clear Dependencys
		clearPipelineLayouts();
		clearRasterizationPipelines();
		clearComputePipelines();
	}
	void clearPipelineLayouts() override
	{
		clear(pipelineLayouts);

		// Clear Dependencys
		clearRasterizationPipelines();
		clearComputePipelines();
	}
	void clearRasterizationPipelines() override
	{
		clear(rasterizationPipelines);
	}
	void clearComputePipelines() override
	{
		clear(computePipelines);
	}
	void clearRenderPasses() override
	{
		clear(renderPasses);

		// Clear Dependencys
		clearRasterizationPipelines();
		clearComputePipelines();
	}
	void clearSamplers() override
	{
		clear(samplers);
	}
	void clearShaders() override
	{
		clear(shaders);

		// Clear Dependencys
		clearRasterizationPipelines();
		clearComputePipelines();
	}
	void clearAll() override
	{
		clearDescSetLayouts();
		clearPipelineLayouts();
		clearRasterizationPipelines();
		clearComputePipelines();
		clearRenderPasses();
		clearSamplers();
		clearShaders();
	}


	ResourceCache(){};
	~ResourceCache(){};
	DELETE_COPY_CONSTRUCTORS(ResourceCache);
};
}        // namespace vka
