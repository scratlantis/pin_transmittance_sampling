#pragma once
#include <vka/core/common.h>

namespace vka
{
class CmdAllocator
{
  public:
	CmdAllocator() = default;
	void            init();
	void			createCmdBuffersUniversal(uint32_t queueIdx, VkCommandBufferLevel cmdBufLevel, uint32_t count, VkCommandBuffer &cmdBuf, VkCommandPool &cmdPool);
	void            createCmdBuffersCompute(uint32_t queueIdx, VkCommandBufferLevel cmdBufLevel, uint32_t count, VkCommandBuffer &cmdBuf, VkCommandPool &cmdPool);
	void            destroy();
	DELETE_COPY_CONSTRUCTORS(CmdAllocator);
private:
	std::vector<VkCommandPool> universalPools;
	std::vector<VkCommandPool> computePools;
};
}        // namespace vka