#pragma once
#include <vka/core/common.h>
#include <unordered_map>
#include <vka/interface/resources.h>
namespace vka
{

class FramebufferCache
{
	std::unordered_map<hash_t, VkFramebuffer> cache;
  public:
	void clear();
	VkFramebuffer fetch(VkRenderPass renderPass, std::vector<VkaImage> images);
};
}