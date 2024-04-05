#pragma once
#include "../core/macros/macros.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace vka
{
class CmdAllocator
{
  public:
	  CmdAllocator() = default;
	void init();
	void destroy();
	DELETE_COPY_CONSTRUCTORS(CmdAllocator);
private:
	std::vector<VkCommandPool> pools;
};
}        // namespace vka