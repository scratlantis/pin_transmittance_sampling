#include "ResourceCache.h"

#include <vka/resource_objects/DescriptorSetLayout.h>
#include <vka/resource_objects/PipelineLayout.h>
#include <vka/resource_objects/Shader.h>
#include <vka/resource_objects/RenderPass.h>
namespace vka
{
DescriptorSetLayout* ResourceCache::fetch(const DescriptorSetLayoutDefinition &rID)
{
	DescriptorSetLayout *res = nullptr;
	auto it = descSetLayouts.find(rID);
	if (it != descSetLayouts.end())
		res = &it->second;
	else
	{
		res = new DescriptorSetLayout(rID);
	}
	return res;
}

PipelineLayout *ResourceCache::fetch(const PipelineLayoutDefinition &rID)
{
	PipelineLayout *res = nullptr;
	auto                 it  = pipelineLayouts.find(rID);
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
	auto                 it  = shaders.find(rID);
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
	auto    it  = renderpasses.find(rID);
	if (it != renderpasses.end())
		res = &it->second;
	else
	{
		res = new RenderPass(rID);
	}
	return res;
}

}        // namespace vka


//void vka::ResourceCache::clear()
//{
//	auto it = resources.begin();
//	for (auto it = resources.begin(); it != resources.end(); ++it)
//	{
//		(*it)->free();
//		delete *it;
//	}
//	resources.clear();
//}