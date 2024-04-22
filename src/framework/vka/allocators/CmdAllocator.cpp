#include "CmdAllocator.h"
#include "../global_state.h"


namespace vka
{
void CmdAllocator::init()
{
	ASSERT_TRUE(gState.initBits & (STATE_INIT_DEVICE_BIT | STATE_INIT_IO_BIT));

	VkCommandPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	universalPools.resize(gState.device.universalQueues.size());
	for (size_t i = 0; i < universalPools.size(); i++)
	{
		poolInfo.queueFamilyIndex = gState.device.universalQueueFamily;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		ASSERT_VULKAN(vkCreateCommandPool(gState.device.logical, &poolInfo, nullptr, &universalPools[i]));
	}

	computePools.resize(gState.device.computeQueues.size());
	for (size_t i = 0; i < computePools.size(); i++)
	{
		poolInfo.queueFamilyIndex = gState.device.computeQueueFamily;
		poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		ASSERT_VULKAN(vkCreateCommandPool(gState.device.logical, &poolInfo, nullptr, &universalPools[i]));
	}

	gState.initBits |= STATE_INIT_CMDALLOC_BIT;
}

void CmdAllocator::createCmdBuffersUniversal(uint32_t queueIdx, VkCommandBufferLevel cmdBufLevel, uint32_t count, VkCommandBuffer &cmdBuf)
{
	VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	allocInfo.commandPool                 = universalPools[queueIdx];
	allocInfo.level						  = cmdBufLevel;
	allocInfo.commandBufferCount          = count;
	ASSERT_VULKAN(vkAllocateCommandBuffers(gState.device.logical, &allocInfo, &cmdBuf));
}

void CmdAllocator::createCmdBuffersCompute(uint32_t queueIdx, VkCommandBufferLevel cmdBufLevel, uint32_t count, VkCommandBuffer &cmdBuf)
{
	VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	allocInfo.commandPool        = computePools[queueIdx];
	allocInfo.level              = cmdBufLevel;
	allocInfo.commandBufferCount = count;
	ASSERT_VULKAN(vkAllocateCommandBuffers(gState.device.logical, &allocInfo, &cmdBuf));
}

void CmdAllocator::destroy()
{
	ASSERT_TRUE(gState.initBits & STATE_INIT_CMDALLOC_BIT);

	for (size_t i = 0; i < universalPools.size(); i++)
	{
		vkDestroyCommandPool(gState.device.logical, universalPools[i], nullptr);
	}
	universalPools.clear();
	for (size_t i = 0; i < computePools.size(); i++)
	{
		vkDestroyCommandPool(gState.device.logical, computePools[i], nullptr);
	}
	computePools.clear();
	gState.initBits &= ~STATE_INIT_CMDALLOC_BIT;
}
}        // namespace vka