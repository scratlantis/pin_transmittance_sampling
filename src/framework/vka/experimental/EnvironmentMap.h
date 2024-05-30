#pragma once
#include "../combined_resources/Image.h"
#include "../combined_resources/CmdBuffer.h"
#include "../resources/Sampler.h"
#include <framework/vka/default/ConfigurableRenderPass.h>

namespace vka
{
class EnvironmentMap : public ImageVma
{
  public:
	EnvironmentMap(ResourceTracker *pTracker, const VkImageCreateInfo &imgCI, VkImageViewCreateInfo &imgViewCI, const VkSamplerCreateInfo &samplerCI, VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY)
		: ImageVma(pTracker, imgCI, memUsage), samplerCI(samplerCI)
	{
		imgViewCI.image = img;
		createImageView(pTracker, imgViewCI);
	};
	~EnvironmentMap(){};


	void render(ComputeCmdBuffer& cmdBuf, Image target, Buffer viewBuffer)
	{
		glm::uvec3           workGroupSize  = {128, 1, 1};
		glm::uvec3           resolution     = {target.extent.width, target.extent.height, 1};
		glm::uvec3           workGroupCount = getWorkGroupCount(workGroupSize, resolution);
		ComputePipelineState computeState{};
		computeState.shaderDef.name = "render_env_map.comp";
		DescriptorSetLayoutDefinition layoutDefinition{};
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_SAMPLER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		layoutDefinition.flags                          = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
		computeState.pipelineLayoutDef.descSetLayoutDef = {layoutDefinition};
		//computeState.pipelineLayoutDef.pcRanges		 = { {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(glm::mat4)} };
		computeState.specialisationEntrySizes           = glm3VectorSizes();
		computeState.specializationData                 = getByteVector(workGroupSize);

		ComputePipeline computePipeline                 = ComputePipeline(&gState.cache, computeState);
		cmdBuf.bindPipeline(computePipeline);
		cmdBuf.pushDescriptors(0, viewBuffer, Sampler(&gState.cache, samplerCI), (Image) * this, (Image) target);
		//cmdBuf.pushConstants(0, sizeof(glm::mat4), &viewProjectionMat);
		cmdBuf.dispatch(workGroupCount);
	}

	Sampler getSampler() const
	{
		return Sampler(&gState.cache, samplerCI);
	}

	  const VkSamplerCreateInfo samplerCI;
  private:
};

}