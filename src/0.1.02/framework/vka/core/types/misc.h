#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <stdint.h>
#include <vka/core/container/misc.h>
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

enum ClearValueType
{
	CLEAR_VALUE_FLOAT,
	CLEAR_VALUE_INT,
	CLEAR_VALUE_UINT,
	DEPTH_STENCIL
};
struct ClearValue
{
	ClearValueType type;
	VkClearValue   value;

	ClearValue(float r, float g, float b, float a)
	{
		type = CLEAR_VALUE_FLOAT;
		value.color.float32[0] = r;
		value.color.float32[1] = g;
		value.color.float32[2] = b;
		value.color.float32[3] = a;
	}
	ClearValue(int32_t r, int32_t g, int32_t b, int32_t a)
	{
		type = CLEAR_VALUE_INT;
		value.color.int32[0] = r;
		value.color.int32[1] = g;
		value.color.int32[2] = b;
		value.color.int32[3] = a;
	}
	ClearValue(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
	{
		type = CLEAR_VALUE_UINT;
		value.color.uint32[0] = r;
		value.color.uint32[1] = g;
		value.color.uint32[2] = b;
		value.color.uint32[3] = a;
	}
	ClearValue(float depth, uint32_t stencil)
	{
		type = DEPTH_STENCIL;
		value.depthStencil.depth   = depth;
		value.depthStencil.stencil = stencil;
	}
	bool operator==(const ClearValue &other) const
	{
		if (type != other.type)
			return false;
		switch (type)
		{
			case CLEAR_VALUE_FLOAT:
				return value.color.float32[0] == other.value.color.float32[0] &&
				       value.color.float32[1] == other.value.color.float32[1] &&
				       value.color.float32[2] == other.value.color.float32[2] &&
				       value.color.float32[3] == other.value.color.float32[3];
			case CLEAR_VALUE_INT:
				return value.color.int32[0] == other.value.color.int32[0] &&
				       value.color.int32[1] == other.value.color.int32[1] &&
				       value.color.int32[2] == other.value.color.int32[2] &&
				       value.color.int32[3] == other.value.color.int32[3];
			case CLEAR_VALUE_UINT:
				return value.color.uint32[0] == other.value.color.uint32[0] &&
				       value.color.uint32[1] == other.value.color.uint32[1] &&
				       value.color.uint32[2] == other.value.color.uint32[2] &&
				       value.color.uint32[3] == other.value.color.uint32[3];
			case DEPTH_STENCIL:
				return value.depthStencil.depth == other.value.depthStencil.depth &&
				       value.depthStencil.stencil == other.value.depthStencil.stencil;
		}
		return false;
	}
};
typedef uint64_t hash_t;
}        // namespace vka