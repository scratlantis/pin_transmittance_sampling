#pragma once
#include "commands.h"
#include "../buffer_functionality.h"
#include "../image_functionality.h"
#include "../cmd_buffer_functionality.h"

using namespace vka;

// Buffer
void vkaCmdCopyBuffer(VkaCommandBuffer cmdBuf, const Buffer_I *src, const Buffer_I *dst)
{
	VkDeviceSize minDataSize = std::min(src->getSize(), dst->getSize());
	VkBufferCopy copyRegion{0, 0, minDataSize};
	vkCmdCopyBuffer(cmdBuf->getHandle(), src->getHandle(), dst->getHandle(), 1, &copyRegion);
}
void vkaCmdUpload(VkaCommandBuffer cmdBuf, VkaBuffer buf)
{
	buf->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
	const Buffer_I localBuf = buf->recreate();
	vkaCmdCopyBuffer(cmdBuf, &localBuf, buf);
}

// Image
void vkaCmdImageMemoryBarrier(VkaCommandBuffer cmdBuf, VkaImage image, VkImageLayout newLayout, uint32_t baseLayer, uint32_t layerCount)
{
	VkImageMemoryBarrier memory_barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
	memory_barrier.oldLayout                       = image->getLayout();
	memory_barrier.newLayout                       = newLayout;
	memory_barrier.image                           = image->getHandle();
	memory_barrier.subresourceRange.aspectMask     = getAspectFlags(image->getFormat());
	memory_barrier.subresourceRange.baseMipLevel   = 0;
	memory_barrier.subresourceRange.levelCount     = image->getMipLevels();
	memory_barrier.subresourceRange.baseArrayLayer = baseLayer;
	memory_barrier.subresourceRange.layerCount     = layerCount;
	memory_barrier.srcAccessMask                   = getAccessFlags(image->getLayout());
	memory_barrier.dstAccessMask                   = getAccessFlags(newLayout);
	VkPipelineStageFlags src_stage                 = getStageFlags(image->getLayout());
	VkPipelineStageFlags dst_stage                 = getStageFlags(newLayout);
	vkCmdPipelineBarrier(
	    cmdBuf->getHandle(),
	    src_stage, dst_stage,
	    0,
	    0, nullptr,
	    0, nullptr,
	    1, &memory_barrier);
	image->setLayout(newLayout);
}
void vkaCmdTransitionLayout(VkaCommandBuffer cmdBuf, VkaImage image, VkImageLayout newLayout, uint32_t baseLayer, uint32_t layerCount)
{
	if (image->getLayout() != newLayout && newLayout != VK_IMAGE_LAYOUT_UNDEFINED && newLayout != VK_IMAGE_LAYOUT_PREINITIALIZED)
	{
		vkaCmdImageMemoryBarrier(cmdBuf, image, newLayout, baseLayer, layerCount);
	}
}
void vkaCmdCopyBufferToImage(VkaCommandBuffer cmdBuf, VkaBuffer src, VkaImage dst, uint32_t layer, uint32_t mipLevel)
{
	VkDeviceSize dstSize = dst->getMemorySize();
	VKA_ASSERT(src->getSize() == dstSize);
	VkBufferImageCopy region{};
	region.bufferOffset                    = 0;
	region.bufferRowLength                 = 0;
	region.bufferImageHeight               = 0;
	region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel       = mipLevel;
	region.imageSubresource.baseArrayLayer = layer;
	region.imageSubresource.layerCount     = 1;
	region.imageOffset                     = {0, 0, 0};
	region.imageExtent                     = dst->getExtent();
	vkCmdCopyBufferToImage(cmdBuf->getHandle(), src->getHandle(), dst->getHandle(), dst->getLayout(), 1, &region);
}

void vkaCmdUploadImageData(VkaCommandBuffer cmdBuf, void *data, size_t dataSize, VkaImage dst, VkImageLayout finalLayout, uint32_t layer, uint32_t mipLevel)
{
	VkaBuffer stagingBuffer = vkaCreateBuffer(gState.frame->stack);
	vkaWriteStaging(stagingBuffer, data, dataSize);
	vkaCmdTransitionLayout(cmdBuf, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layer, 1);
	vkaCmdCopyBufferToImage(cmdBuf, stagingBuffer, dst, layer, mipLevel);
	vkaCmdTransitionLayout(cmdBuf, dst, finalLayout, layer, 1);
}

void vkaCmdCopyImage(VkaCommandBuffer cmdBuf, VkaImage src, VkImageLayout srcNewLayout, VkaImage dst, VkImageLayout dstNewLayout,
                     ImageSubresourceRange srcSubRange, ImageSubresourceRange dstSubRange)
{
	vkaCmdTransitionLayout(cmdBuf, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcSubRange.baseArrayLayer, srcSubRange.layerCount);
	vkaCmdTransitionLayout(cmdBuf, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstSubRange.baseArrayLayer, dstSubRange.layerCount);

	VkImageSubresourceLayers subresourceLayerSrc{};
	subresourceLayerSrc.aspectMask     = getAspectFlags(src->getFormat());
	subresourceLayerSrc.mipLevel       = srcSubRange.mipLevel;
	subresourceLayerSrc.baseArrayLayer = srcSubRange.baseArrayLayer;
	subresourceLayerSrc.layerCount     = srcSubRange.layerCount;

	VkImageSubresourceLayers subresourceLayerDst{};
	subresourceLayerDst.aspectMask     = getAspectFlags(dst->getFormat());
	subresourceLayerDst.mipLevel       = dstSubRange.mipLevel;
	subresourceLayerDst.baseArrayLayer = dstSubRange.baseArrayLayer;
	subresourceLayerDst.layerCount     = dstSubRange.layerCount;

	VkOffset3D offset{};
	offset.x = 0;
	offset.y = 0;
	offset.z = 0;

	VkImageCopy imageCopy{};
	imageCopy.srcSubresource = subresourceLayerSrc;
	imageCopy.srcOffset      = offset;
	imageCopy.dstSubresource = subresourceLayerDst;
	imageCopy.dstOffset      = offset;
	imageCopy.extent         = src->getExtent();

	vkCmdCopyImage(cmdBuf->getHandle(), src->getHandle(), src->getLayout(), dst->getHandle(), dst->getLayout(), 1, &imageCopy);

	vkaCmdTransitionLayout(cmdBuf, src, srcNewLayout, srcSubRange.baseArrayLayer, srcSubRange.layerCount);
	vkaCmdTransitionLayout(cmdBuf, dst, dstNewLayout, dstSubRange.baseArrayLayer, dstSubRange.layerCount);
}

void vkaCmdCopyImage(VkaCommandBuffer cmdBuf, VkaImage src, VkaImage dst)
{
	vkaCmdCopyImage(cmdBuf, src, src->getLayout(), dst, dst->getLayout());
}

// General
void barrier(VkaCommandBuffer cmdBuf, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkAccessFlags srcAccesFlags, VkAccessFlags dstAccesFlags)
{
	VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
	barrier.srcAccessMask = srcAccesFlags;
	barrier.dstAccessMask = dstAccesFlags;
	vkCmdPipelineBarrier(cmdBuf->getHandle(), srcStage, dstStage, 0, 1, &barrier, 0, nullptr, 0, nullptr);
}

void vkaCmdFillBuffer(VkaCommandBuffer cmdBuf, VkaBuffer dst, VkDeviceSize offset, VkDeviceSize size, uint32_t data)
{
	vkCmdFillBuffer(cmdBuf->getHandle(), dst->getHandle(), offset, size, data);
}

void vkaCmdFillBuffer(VkaCommandBuffer cmdBuf, VkaBuffer dst, uint32_t data)
{
	vkCmdFillBuffer(cmdBuf->getHandle(), dst->getHandle(), 0, VK_WHOLE_SIZE, data);
}

// Pipelines, render/dispatch
void vkaCmdStartRenderPass(VkaCommandBuffer cmdBuf, VkRenderPass renderpass, VkFramebuffer framebuffer, std::vector<VkClearValue> clearValues, VkRect2D renderArea)
{
	VkCommandBuffer       handle = cmdBuf->getHandle();
	VkRenderPassBeginInfo renderPassBeginInfo;
	renderPassBeginInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext           = nullptr;
	renderPassBeginInfo.renderPass      = renderpass;
	renderPassBeginInfo.framebuffer     = framebuffer;
	renderPassBeginInfo.renderArea      = renderArea;
	renderPassBeginInfo.clearValueCount = clearValues.size();
	renderPassBeginInfo.pClearValues    = clearValues.data();

	VkExtent2D extent = renderArea.extent;
	vkCmdBeginRenderPass(handle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	VkViewport viewport;
	viewport.x        = renderArea.offset.x;
	viewport.y        = renderArea.offset.y;
	viewport.width    = extent.width;
	viewport.height   = extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(handle, 0, 1, &viewport);
	VkRect2D scissor;
	scissor.offset = renderArea.offset;
	scissor.extent = {extent.width, extent.height};
	vkCmdSetScissor(handle, 0, 1, &scissor);
}

void vkaCmdEndRenderPass(VkaCommandBuffer cmdBuf)
{
	vkCmdEndRenderPass(cmdBuf->getHandle());
}

// bind, dispatch/draw, push desc, push constants

void vkaCmdBindPipeline(VkaCommandBuffer cmdBuf, ComputePipelineDefinition def)
{
	cmdBuf->renderState.pipeline          = gState.cache->fetch(def);
	cmdBuf->renderState.pipelineLayoutDef = def.pipelineLayoutDefinition;
	cmdBuf->renderState.bindPoint         = VK_PIPELINE_BIND_POINT_COMPUTE;
	vkCmdBindPipeline(cmdBuf->getHandle(), cmdBuf->renderState.bindPoint, cmdBuf->renderState.pipeline);
	cmdBuf->stateBits |= CMD_BUF_STATE_BITS_BOUND_PIPELINE;
}

void vkaCmdBindPipeline(VkaCommandBuffer cmdBuf)
{
	vkCmdBindPipeline(cmdBuf->getHandle(), cmdBuf->renderState.bindPoint, cmdBuf->renderState.pipeline);
	cmdBuf->stateBits |= CMD_BUF_STATE_BITS_BOUND_PIPELINE;
}

void vkaCmdPushDescriptors(VkaCommandBuffer cmdBuf, uint32_t setIdx, std::vector<IDescriptor *> desc)
{
	LOAD_CMD_VK_DEVICE(vkCmdPushDescriptorSetKHR, gState.device.logical);
	VKA_ASSERT(cmdBuf->stateBits & CMD_BUF_STATE_BITS_BOUND_PIPELINE);
	VKA_ASSERT(cmdBuf->renderState.pipelineLayoutDef.descSetLayoutDef.size() > setIdx);

	DescriptorSetLayoutDefinition       descLayoutDef = cmdBuf->renderState.pipelineLayoutDef.descSetLayoutDef[setIdx];
	std::vector<VkWriteDescriptorSet>   writes(VKA_COUNT(desc));
	std::vector<VkDescriptorBufferInfo> bufferInfos(VKA_COUNT(desc));
	std::vector<VkDescriptorImageInfo>  imageInfos(VKA_COUNT(desc));
	VkDescriptorBufferInfo             *pBuffInfo  = &bufferInfos[0];
	VkDescriptorImageInfo              *pImageInfo = &imageInfos[0];
	for (size_t i = 0; i < writes.size(); i++)
	{
		writes[i]                 = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
		writes[i].dstBinding      = i;
		writes[i].descriptorCount = 1;
		writes[i].descriptorType  = descLayoutDef.bindings[i].descriptorType;
		desc[i]->writeDescriptorInfo(writes[i], pBuffInfo, pImageInfo);
	}
	pvkCmdPushDescriptorSetKHR(cmdBuf->getHandle(), cmdBuf->renderState.bindPoint, gState.cache->fetch(cmdBuf->renderState.pipelineLayoutDef), setIdx, writes.size(), writes.data());
}

void vkaCmdPushConstants(VkaCommandBuffer cmdBuf, VkShaderStageFlags shaderStage, uint32_t offset, uint32_t size, const void *data)
{
	VKA_ASSERT(cmdBuf->stateBits & CMD_BUF_STATE_BITS_BOUND_PIPELINE);
	vkCmdPushConstants(cmdBuf->getHandle(), gState.cache->fetch(cmdBuf->renderState.pipelineLayoutDef), shaderStage, offset, size, data);
}

void vkaCmdDispatch(VkaCommandBuffer cmdBuf, uint32_t x, uint32_t y, uint32_t z)
{
	VKA_ASSERT(cmdBuf->stateBits & CMD_BUF_STATE_BITS_BOUND_PIPELINE);
	vkCmdDispatch(cmdBuf->getHandle(), x, y, z);
}

void vkaCmdBindVertexBuffers(VkaCommandBuffer cmdBuf)
{
	std::vector<VkBuffer> handels;
	for (size_t i = 0; i < cmdBuf->renderState.vertexBuffers.size(); i++)
	{
		handels.push_back(cmdBuf->renderState.vertexBuffers[i]->getHandle());
	}
	VkDeviceSize offset(handels.size());
	vkCmdBindVertexBuffers(cmdBuf->getHandle(), 0, 1, handels.data(), &offset);
}

void vkaCmdBindIndexBuffer(VkaCommandBuffer cmdBuf, VkDeviceSize offset)
{
	vkCmdBindIndexBuffer(cmdBuf->getHandle(), cmdBuf->renderState.indexBuffer->getHandle(), offset, VK_INDEX_TYPE_UINT32);
}

void vkaCmdDrawIndexed(VkaCommandBuffer cmdBuf, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	vkCmdDrawIndexed(cmdBuf->getHandle(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void vkaCmdDraw(VkaCommandBuffer cmdBuf, DrawCmd &drawCall)
{
	VKA_ASSERT(drawCall.model.surfaceCount == 1);
	VKA_ASSERT(drawCall.pipelineDef.subpass == 0);

	RenderState newRenderState = drawCall.getRenderState();
	uint32_t    diffBits       = cmdBuf->renderState.calculateDifferenceBits(newRenderState);
	cmdBuf->renderState        = newRenderState;
	if (diffBits & RENDER_STATE_ACTION_BIT_END_RENDER_PASS)
	{
		vkaCmdEndRenderPass(cmdBuf);
	}

	if (diffBits & RENDER_STATE_ACTION_BIT_START_RENDER_PASS)
	{
		VKA_ASSERT(drawCall.attachments.size() == drawCall.pipelineDef.renderPassDefinition.attachmentDescriptions.size())
		for (size_t i = 0; i < drawCall.attachments.size(); i++)
		{
			vkaCmdTransitionLayout(cmdBuf, drawCall.attachments[i], drawCall.pipelineDef.renderPassDefinition.attachmentDescriptions[i].initialLayout);
			drawCall.attachments[i]->setLayout(drawCall.pipelineDef.renderPassDefinition.attachmentDescriptions[i].finalLayout);
		}
		vkaCmdStartRenderPass(cmdBuf, cmdBuf->renderState.renderPass, cmdBuf->renderState.framebuffer, cmdBuf->renderState.getClearValues(), newRenderState.renderArea);
	}

	if (diffBits & RENDER_STATE_ACTION_BIT_BIND_PIPELINE)
	{
		vkaCmdBindPipeline(cmdBuf);
	}
	// for now, only one descriptor set, always rebind
	vkaCmdPushDescriptors(cmdBuf, 0, drawCall.descriptors);

	if (diffBits & RENDER_STATE_ACTION_BIT_BIND_VERTEX_BUFFER)
	{
		vkaCmdBindVertexBuffers(cmdBuf);
	}

	// for now only one surface
	SurfaceData surfaceData[1];
	if (drawCall.model.surfaceBuffer)
	{
		vkaRead(drawCall.model.surfaceBuffer, &surfaceData[0]);
	}
	else
	{
		surfaceData[0].indexOffset = 0;
		surfaceData[0].indexCount  = drawCall.model.indexBuffer->getSize() / sizeof(Index);
		surfaceData[0].vertexOffset = 0;
		surfaceData[0].vertexCount = drawCall.model.vertexBuffer->getSize() / drawCall.pipelineDef.vertexBindingDescriptions[0].stride;
	}

	if (diffBits & RENDER_STATE_ACTION_BIT_BIND_INDEX_BUFFER)
	{
		vkaCmdBindIndexBuffer(cmdBuf, surfaceData[0].indexOffset);
	}

	vkaCmdDrawIndexed(cmdBuf, surfaceData[0].indexCount, drawCall.instanceCount, surfaceData[0].indexOffset, surfaceData[0].vertexOffset, 0);
}

void vkaCmdFinishDraw(VkaCommandBuffer cmdBuf)
{
	vkaCmdEndRenderPass(cmdBuf);
	cmdBuf->renderState = {};
}