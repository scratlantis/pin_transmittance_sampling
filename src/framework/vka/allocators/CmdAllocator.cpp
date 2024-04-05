#include "CmdAllocator.h"
#include "../global_state.h"


namespace vka
{
void CmdAllocator::init()
{
	ASSERT_TRUE(gState.initBits & (STATE_INIT_DEVICE_BIT | STATE_INIT_IO_BIT));

	/*pools.resize(threadCount);
	VkCommandPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	for (size_t i = 0; i < threadCount; i++)
	{
	    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	    poolInfo.queueFamilyIndex = gState.queueFamilyIndex;
	    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	    VK_CHECK(vkCreateCommandPool(gState.device, &poolInfo, nullptr, &pools[i]));

	}*/
	gState.initBits |= STATE_INIT_CMDALLOC_BIT;
}

void CmdAllocator::destroy()
{
	ASSERT_TRUE(gState.initBits & STATE_INIT_CMDALLOC_BIT);
	gState.initBits &= ~STATE_INIT_CMDALLOC_BIT;
}
}        // namespace vka