#include "ResourceCache.h"

#include <vka/resource_objects/DescriptorSetLayout.h>
#include <vka/resource_objects/PipelineLayout.h>
#include <vka/resource_objects/RasterizationPipeline.h>
#include <vka/resource_objects/ComputePipeline.h>
#include <vka/resource_objects/RenderPass.h>
#include <vka/resource_objects/Sampler.h>
#include <vka/resource_objects/Shader.h>

namespace vka
{
DescriptorSetLayout *ResourceCache::fetch(const DescriptorSetLayoutDefinition &rID)
{
	DescriptorSetLayout *res = nullptr;
	auto                 it  = descSetLayouts.find(rID);
	if (it != descSetLayouts.end())
		res = &it->second;
	else
	{
		res = new DescriptorSetLayout(rID);
		descSetLayouts.insert({rID, *res});
	}
	return res;
}

PipelineLayout *ResourceCache::fetch(const PipelineLayoutDefinition &rID)
{
	PipelineLayout *res = nullptr;
	auto            it  = pipelineLayouts.find(rID);
	if (it != pipelineLayouts.end())
		res = &it->second;
	else
	{
		res = new PipelineLayout(rID);
	}
	return res;
}

Shader *ResourceCache::fetch(const ShaderDefinition &rID)
{
	Shader *res = nullptr;
	auto    it  = shaders.find(rID);
	if (it != shaders.end())
		res = &it->second;
	else
	{
		res = new Shader(rID);
	}
	return res;
}

RenderPass *ResourceCache::fetch(const RenderPassDefinition &rID)
{
	RenderPass *res = nullptr;
	auto        it  = renderPasses.find(rID);
	if (it != renderPasses.end())
		res = &it->second;
	else
	{
		res = new RenderPass(rID);
	}
	return res;
}

RasterizationPipeline *ResourceCache::fetch(const RasterizationPipelineDefinition &rID)
{
	RasterizationPipeline *res = nullptr;
	auto                   it  = rasterizationPipelines.find(rID);
	if (it != rasterizationPipelines.end())
		res = &it->second;
	else
	{
		res = new RasterizationPipeline(rID);
	}
	return res;
}

ComputePipeline *ResourceCache::fetch(const ComputePipelineDefinition &rID)
{
	ComputePipeline *res = nullptr;
	auto             it  = computePipelines.find(rID);
	if (it != computePipelines.end())
		res = &it->second;
	else
	{
		res = new ComputePipeline(rID);
	}
	return res;
}

Sampler *ResourceCache::fetch(const SamplerDefinition &rID)
{
	Sampler *res = nullptr;
	auto     it  = samplers.find(rID);
	if (it != samplers.end())
		res = &it->second;
	else
	{
		res = new Sampler(rID);
	}
	return res;
}


VkDescriptorSetLayout ResourceCache::fetchHandle(DescriptorSetLayoutDefinition const &rID)
{
	return fetch(rID)->getHandle();
};
VkPipelineLayout ResourceCache::fetchHandle(PipelineLayoutDefinition const &rID)
{
	return fetch(rID)->getHandle();
};
VkShaderModule ResourceCache::fetchHandle(ShaderDefinition const &rID)
{
	return fetch(rID)->getHandle();
};
VkRenderPass ResourceCache::fetchHandle(RenderPassDefinition const &rID)
{
	return fetch(rID)->getHandle();
};
VkPipeline ResourceCache::fetchHandle(RasterizationPipelineDefinition const &rID)
{
	return fetch(rID)->getHandle();
};
VkPipeline ResourceCache::fetchHandle(ComputePipelineDefinition const &rID)
{
	return fetch(rID)->getHandle();
};
VkSampler ResourceCache::fetchHandle(SamplerDefinition const &rID)
{
	return fetch(rID)->getHandle();
};

//void ResourceCache::clear_descSetLayouts()
//{
//	auto it = descSetLayouts.begin();
//	for (auto it = descSetLayouts.begin(); it != descSetLayouts.end(); ++it)
//	{
//		(*it)->free();
//		delete *it;
//	}
//	descSetLayouts.clear();
// }

}        // namespace vka
