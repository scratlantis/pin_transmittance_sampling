#include "Descriptor.h"

namespace vka
{
Descriptor::Descriptor(const Buffer_R *buffer, VkDescriptorType type, VkShaderStageFlags shaderStage)
{
	VKA_ASSERT(buffer != nullptr &&
	           (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
	            type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
	            type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
	            type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC));
	stage = shaderStage;
	count = 1;
	buffers = {buffer};
	this->type = type;
}
Descriptor::Descriptor(const Image_R *image, VkDescriptorType type, VkShaderStageFlags shaderStage)
{
	VKA_ASSERT(image != nullptr &&
	           (type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
	            type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE));
	stage      = shaderStage;
	count      = 1;
	images     = {image};
	this->type = type;
}
Descriptor::Descriptor(const SamplerDefinition samplerDef, VkShaderStageFlags shaderStage)
{
	stage    = shaderStage;
	count = 1;
	samplers = {gState.cache->fetch(samplerDef)};
	type = VK_DESCRIPTOR_TYPE_SAMPLER;
}
Descriptor::Descriptor(std::vector<const Buffer_R *> buffers, VkDescriptorType type, VkShaderStageFlags shaderStage)
{
	VKA_ASSERT(type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
	           type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
	           type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
	           type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
	this->buffers = buffers;
	this->type    = type;
	this->stage   = shaderStage;
	count         = buffers.size();
}
Descriptor::Descriptor(std::vector<const Image_R *> images, VkDescriptorType type, VkShaderStageFlags shaderStage)
{
	VKA_ASSERT(type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
	           type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	this->images = images;
	this->type   = type;
	this->stage  = shaderStage;
	count        = images.size();
}
Descriptor::Descriptor(std::vector<SamplerDefinition> samplersDefs, VkShaderStageFlags shaderStage)
{
	this->type = VK_DESCRIPTOR_TYPE_SAMPLER;
	this->stage = shaderStage;
	count = samplersDefs.size();
	for (auto &def : samplersDefs)
	{
		samplers.push_back(gState.cache->fetch(def));
	}
}
Descriptor::Descriptor(SamplerDefinition samplerDef, const Image_R *image, VkShaderStageFlags shaderStage)
{
	VKA_ASSERT(image != nullptr);
	stage    = shaderStage;
	count = 1;
	images = {image};
	samplers = {gState.cache->fetch(samplerDef)};
	type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
}

void Descriptor::writeDescriptorInfo(VkWriteDescriptorSet &write, VkDescriptorBufferInfo *&pBufferInfo, VkDescriptorImageInfo *&pImageInfos) const
{
	if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
	{
		VKA_ASSERT(buffers.size() == count);
		write.descriptorCount = count;
		write.pBufferInfo   = pBufferInfo;
		write.descriptorType = type;
		for (uint32_t i = 0; i < count; i++)
		{
			pBufferInfo->buffer = buffers[i]->getHandle();
			pBufferInfo->offset = buffers[i]->getRange().offset;
			pBufferInfo->range  = buffers[i]->getRange().size;
			pBufferInfo++;
		}
		return;
	}
	if (type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
	{
		VKA_ASSERT(images.size() == count);
		write.descriptorCount = count;
		write.pImageInfo      = pImageInfos;
		write.descriptorType = type;
		for (uint32_t i = 0; i < count; i++)
		{
			pImageInfos->imageView   = images[i]->getViewHandle();
			pImageInfos->imageLayout = images[i]->getLayout();
			pImageInfos->sampler     = nullptr;
			pImageInfos++;
		}
		return;
	}
	if (type == VK_DESCRIPTOR_TYPE_SAMPLER)
	{
		VKA_ASSERT(samplers.size() == count);
		write.descriptorCount = count;
		write.pImageInfo      = pImageInfos;
		write.descriptorType  = type;
		for (uint32_t i = 0; i < count; i++)
		{
			pImageInfos->imageView   = VK_NULL_HANDLE;
			pImageInfos->imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			pImageInfos->sampler     = samplers[i];
			pImageInfos++;
		}
		return;
	}
	if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		{
		VKA_ASSERT(images.size() == count);
		VKA_ASSERT(samplers.size() == count);
		write.descriptorCount = count;
		write.pImageInfo      = pImageInfos;
		write.descriptorType  = type;
		for (uint32_t i = 0; i < count; i++)
		{
			pImageInfos->imageView   = images[i]->getViewHandle();
			pImageInfos->imageLayout = images[i]->getLayout();
			pImageInfos->sampler     = samplers[i];
			pImageInfos++;
		}
		return;
	}
	printVka("Invalid descriptor type\n");
	DEBUG_BREAK;
}
}        // namespace vka