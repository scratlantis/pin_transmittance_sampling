#pragma once
#include <vulkan/vulkan.h>
namespace vka
{
class IDescriptor
{
  public:
	virtual void writeDescriptorInfo(VkWriteDescriptorSet &write, VkDescriptorBufferInfo *&pBufferInfo, VkDescriptorImageInfo *&pImageInfos) const = 0;
	//virtual void updateLayout(VkaCommandBuffer cmdBuf, VkDescriptorType type){};
};
}        // namespace vka