#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <stdint.h>
namespace vka
{
struct SubmitSynchronizationInfo
{
	std::vector<VkSemaphore>          waitSemaphores{};
	std::vector<VkSemaphore>          signalSemaphores{};
	std::vector<VkPipelineStageFlags> waitDstStageMask{};
	VkFence                           signalFence = VK_NULL_HANDLE;
};

struct VulkanFormatInfo
{
	uint32_t size;
	uint32_t channel_count;
};


typedef uint64_t hash_t;
}        // namespace vka