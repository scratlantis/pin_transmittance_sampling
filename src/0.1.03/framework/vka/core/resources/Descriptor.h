#pragma once
#include <vka/globals.h>
#include "unique/Buffer.h"
#include "unique/Image.h"
#include "cachable/Sampler.h"
namespace vka
{

class Descriptor
{
  public:
	Descriptor(){};
	Descriptor(const Buffer_R *buffer, VkDescriptorType type, VkShaderStageFlags shaderStage);
	Descriptor(const Image_R *image, VkDescriptorType type, VkShaderStageFlags shaderStage);
	Descriptor(const SamplerDefinition samplerDef, VkShaderStageFlags shaderStage);

	Descriptor(std::vector<const Buffer_R *> buffers, VkDescriptorType type, VkShaderStageFlags shaderStage);
	Descriptor(std::vector<const Image_R *> images, VkDescriptorType type, VkShaderStageFlags shaderStage);
	Descriptor(std::vector < const SamplerDefinition> samplersDefs, VkShaderStageFlags shaderStage);

	Descriptor(const SamplerDefinition samplerDef, const Image_R *image, VkShaderStageFlags shaderStage);
	~Descriptor(){};

  private:
	VkDescriptorType			  type;
	VkShaderStageFlags            stage;
	uint32_t                      count;
	std::vector<const Buffer_R *> buffers;
	std::vector<const Image_R *>  images;
	std::vector<VkSampler>        samplers;

	void       writeDescriptorInfo(VkWriteDescriptorSet &write, VkDescriptorBufferInfo *&pBufferInfo, VkDescriptorImageInfo *&pImageInfos) const;
};
}        // namespace vka