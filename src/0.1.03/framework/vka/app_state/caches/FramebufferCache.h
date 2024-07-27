#pragma once
#include <unordered_map>
#include <vka/core_interface/types.h>
namespace vka
{

class FramebufferCache
{
	std::unordered_map<hash_t, VkFramebuffer> cache;

  public:
	void          clear();
	VkFramebuffer fetch(VkRenderPass renderPass, std::vector<Image> images);
};
}        // namespace vka