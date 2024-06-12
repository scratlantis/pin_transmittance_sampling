#pragma once
#include <vka/state_objects/global_state.h>
//#include <vka/resource_objects/resource_common.h>
#include "../types.h"


// Buffer
void vkaCmdCopyBuffer(VkaCommandBuffer cmdBuf, const vka::Buffer_I *src, const vka::Buffer_I *dst);
void vkaCmdUpload(VkaCommandBuffer cmdBuf, VkaBuffer buf);
void vkaCmdFillBuffer(VkaCommandBuffer cmdBuf, VkaBuffer dst, VkDeviceSize offset, VkDeviceSize size, uint32_t data);
void vkaCmdFillBuffer(VkaCommandBuffer cmdBuf, VkaBuffer dst, uint32_t data);
// Image
void vkaCmdImageMemoryBarrier(VkaCommandBuffer cmdBuf, VkaImage image, VkImageLayout newLayout, uint32_t baseLayer = 0, uint32_t layerCount = 1);
void vkaCmdTransitionLayout(VkaCommandBuffer cmdBuf, VkaImage image, VkImageLayout newLayout, uint32_t baseLayer = 0, uint32_t layerCount = 1);
void vkaCmdCopyBufferToImage(VkaCommandBuffer cmdBuf, VkaBuffer src, VkaImage dst, uint32_t layer = 0, uint32_t mipLevel = 0);
void vkaCmdUploadImageData(VkaCommandBuffer cmdBuf, void *data, size_t dataSize, VkaImage dst, VkImageLayout finalLayout, uint32_t layer = 0, uint32_t mipLevel = 0);
void vkaCmdCopyImage(VkaCommandBuffer cmdBuf, VkaImage src, VkImageLayout srcNewLayout, VkaImage dst, VkImageLayout dstNewLayout,
                     ImageSubresourceRange srcSubRange = {0, 0, 1}, ImageSubresourceRange dstSubRange = {0, 0, 1});
void vkaCmdCopyImage(VkaCommandBuffer cmdBuf, VkaImage src, VkaImage dst);
// General
void barrier(VkaCommandBuffer cmdBuf, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkAccessFlags srcAccesFlags, VkAccessFlags dstAccesFlags);

// Pipelines
void vkaCmdStartRenderPass(VkaCommandBuffer cmdBuf, VkRenderPass renderpass, VkFramebuffer framebuffer,
	std::vector<VkClearValue> clearValues = {}, VkRect2D renderArea = {{0, 0}, gState.io.extent});
void vkaCmdEndRenderPass(VkaCommandBuffer cmdBuf);
void vkaCmdBindPipeline(VkaCommandBuffer cmdBuf, vka::ComputePipelineDefinition def);
void vkaCmdBindPipeline(VkaCommandBuffer cmdBuf);
void vkaCmdPushDescriptors(VkaCommandBuffer cmdBuf, uint32_t setIdx, std::vector<vka::IDescriptor *> desc);
void vkaCmdPushConstants(VkaCommandBuffer cmdBuf, VkShaderStageFlags shaderStage, uint32_t offset, uint32_t size, const void *data);
void vkaCmdDispatch(VkaCommandBuffer cmdBuf, uint32_t x, uint32_t y, uint32_t z);
void vkaCmdBindVertexBuffers(VkaCommandBuffer cmdBuf);
void vkaCmdBindIndexBuffer(VkaCommandBuffer cmdBuf, VkDeviceSize offset);
void vkaCmdDrawIndexed(VkaCommandBuffer cmdBuf, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
void vkaCmdDraw(VkaCommandBuffer cmdBuf, DrawCmd &drawCall);
void vkaCmdFinishDraw(VkaCommandBuffer cmdBuf);

