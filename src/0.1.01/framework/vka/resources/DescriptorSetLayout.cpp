#include "DescriptorSetLayout.h"
#include "../combined_resources/Image.h"
namespace vka
{
DescriptorSetLayout::DescriptorSetLayout(ResourceTracker *pTracker, const DescriptorSetLayoutDefinition &definition) :
    UniqueResource(pTracker), definition(definition)
{
	VkDescriptorSetLayout handle = VK_NULL_HANDLE;
}

DescriptorSetLayout::~DescriptorSetLayout()
{
}



void DescriptorSetLayoutDefinition::addStorageImage(VkShaderStageFlags shaderStage)
{
	VkDescriptorSetLayoutBinding imgBinding{};
	imgBinding.binding            = VKA_COUNT(bindings);
	imgBinding.descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	imgBinding.descriptorCount    = 1;
	imgBinding.stageFlags         = shaderStage;
	imgBinding.pImmutableSamplers = nullptr;
	bindings.push_back(imgBinding);
}

void DescriptorSetLayoutDefinition::addUniformBuffer(VkShaderStageFlags shaderStage)
{
	VkDescriptorSetLayoutBinding uboBinding{};
	uboBinding.binding            = VKA_COUNT(bindings);
	uboBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboBinding.descriptorCount    = 1;
	uboBinding.stageFlags         = shaderStage;
	uboBinding.pImmutableSamplers = nullptr;
	bindings.push_back(uboBinding);
}

void DescriptorSetLayoutDefinition::addDescriptor(VkShaderStageFlags shaderStage, VkDescriptorType type)
{
	VkDescriptorSetLayoutBinding uboBinding{};
	uboBinding.binding            = VKA_COUNT(bindings);
	uboBinding.descriptorType     = type;
	uboBinding.descriptorCount    = 1;
	uboBinding.stageFlags         = shaderStage;
	uboBinding.pImmutableSamplers = nullptr;
	bindings.push_back(uboBinding);
}

}        // namespace vka