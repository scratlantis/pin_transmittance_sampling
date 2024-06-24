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

  public:
	FastDrawState(FramebufferCache* framebufferCache, IResourceCache* resourceCache) : framebufferCache(framebufferCache), resourceCache(resourceCache) {}

	void drawRect(VkaCommandBuffer cmdBuf, VkaImage dst, glm::vec4 color, VkRect2D_OP area);

	void computeHistogram(VkaCommandBuffer cmdBuf, VkaImage src, SamplerDefinition *pSamplerDef, VkaBuffer dst, VkaBuffer average, VkRect2D_OP area);

	void renderHistogram(VkaCommandBuffer cmdBuf, VkaBuffer src, VkaBuffer average, VkaImage dst, VkRect2D_OP area);


};