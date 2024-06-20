#pragma once
#pragma once
#include <vka/interface/types.h>
#include <vka/resource_objects/resource_common.h>
#include <vka/render/tools/FramebufferCache.h>
#


class FastDrawState
{
	FramebufferCache* framebufferCache;
	IResourceCache* resourceCache;

	FastDrawState(FramebufferCache* framebufferCache, IResourceCache* resourceCache) : framebufferCache(framebufferCache), resourceCache(resourceCache) {}

	void drawRect(VkaCommandBuffer cmdBuf, VkImage dst, glm::vec4 color, VkRect2D_OP area);
};