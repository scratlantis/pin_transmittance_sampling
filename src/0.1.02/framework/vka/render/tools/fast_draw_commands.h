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

	void drawEnvMap(VkaCommandBuffer cmdBuf, VkaImage dst, VkaImage envMap, SamplerDefinition samplerDef, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, VkRect2D_OP area);

	void computeHistogram(VkaCommandBuffer cmdBuf, VkaImage src, SamplerDefinition *pSamplerDef, VkaBuffer dst, VkaBuffer average, VkRect2D_OP area);

	void renderHistogram(VkaCommandBuffer cmdBuf, VkaBuffer src, VkaBuffer average, VkaImage dst, VkRect2D_OP area);

	void accumulate(VkaCommandBuffer cmdBuf, VkaImage src, SamplerDefinition sampler, VkaImage dst, VkRect2D_OP area, uint32_t accumulationCount);

	void renderSprite(VkaCommandBuffer cmdBuf, VkaImage src, SamplerDefinition samplerDef, VkaImage dst, VkRect2D_OP area);

	void normalize(VkaCommandBuffer cmdBuf, VkaBuffer buffer, uint32_t count);

	void marginalize(VkaCommandBuffer cmdBuf, VkaBuffer pdfHorizontal, VkaBuffer pdfVertical, VkaImage src, glm::uvec2 binCount);

	void renderDistribution(VkaCommandBuffer cmdBuf, VkaBuffer src, uint32_t binCount, VkaImage dst, VkRect2D_OP area);



};