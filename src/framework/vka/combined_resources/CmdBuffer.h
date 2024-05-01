#pragma once
#include "../resources/Resource.h"
#include "../resources/ComputePipeline.h"
#include "../combined_resources/Buffer.h"
#include "../combined_resources/Image.h"
#include "../compatibility.h"

namespace vka
{

enum CmdBufferStateBits
{
	CMD_BUF_STATE_IS_RECORDING = 0x1,
	CMD_BUF_STATE_BOUND_PIPELINE = 0x2
};

class CmdBuffer;
void commitCmdBuffers(std::vector<CmdBuffer> cmdBufs, ResourceTracker *pTracker, VkQueue queue, const SubmitSynchronizationInfo syncInfo = {});



class CmdBuffer
{
  protected:
	NonUniqueResource *res;
	VkCommandBuffer handle;
	ResourceTracker *pTracker;
	uint32_t        stateBits;
  public:
	CmdBuffer();
	~CmdBuffer();


	void barrier(VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkAccessFlags srcAccesFlags, VkAccessFlags dstAccesFlags)
	{
		VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
		barrier.srcAccessMask = srcAccesFlags;
		barrier.dstAccessMask = dstAccesFlags;
		vkCmdPipelineBarrier(handle, srcStage, dstStage,
		                     0, 1, &barrier, 0, nullptr, 0, nullptr);	}

	template <class T>
	void uploadData(T dataStruct, Buffer dst, ResourceTracker *garbageTracker = &gState.frame->stack)
	{
	    VkDeviceSize size = sizeof(T);
	    uploadData(dst, 0, &dataStruct, size, garbageTracker);
	}

	template <class T>
	Buffer uploadData(T dataStruct, VkBufferUsageFlags usageFlags, ResourceTracker *bufferTracker, ResourceTracker *garbageTracker = &gState.frame->stack)
	{
	    VkDeviceSize size = sizeof(T);
	    Buffer       dst  = BufferVma(bufferTracker, size, usageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	    uploadData(dataStruct, dst);
	    return dst;
	}

	void uploadData(const void* data, size_t size, Buffer dst, ResourceTracker *garbageTracker = &gState.frame->stack)
	{
		uploadData(dst, 0, data, size, garbageTracker);
	}

	Buffer uploadData(const void *data, size_t size, VkBufferUsageFlags usageFlags, ResourceTracker *bufferTracker, ResourceTracker *garbageTracker = &gState.frame->stack)
	{
		Buffer       dst  = BufferVma(bufferTracker, size, usageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		uploadData(data, size, dst);
		return dst;
	}

	void uploadData(Buffer dst, uint32_t dstOffset, const void *data, VkDeviceSize size, ResourceTracker *garbageTracker = &gState.frame->stack)
	{
		Buffer stagingBuf = BufferVma(garbageTracker, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		stagingBuf.write(data, size);
		VkBufferCopy copyRegion{0, dstOffset, size};
		vkCmdCopyBuffer(handle, stagingBuf.buf, dst.buf, 1, &copyRegion);
	}

	void imageMemoryBarrier(Image &image, VkImageLayout newLayout, uint32_t baseLayer = 0, uint32_t layerCount = 1)
	{
		VkImageMemoryBarrier memory_barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
		memory_barrier.oldLayout                       = image.layout;
		memory_barrier.newLayout                       = newLayout;
		memory_barrier.image                           = image.img;
		memory_barrier.subresourceRange.aspectMask     = getAspectFlags(image.format);
		memory_barrier.subresourceRange.baseMipLevel   = 0;
		memory_barrier.subresourceRange.levelCount     = image.mipLevels;
		memory_barrier.subresourceRange.baseArrayLayer = baseLayer;
		memory_barrier.subresourceRange.layerCount     = layerCount;
		memory_barrier.srcAccessMask                   = getAccessFlags(image.layout);
		memory_barrier.dstAccessMask                   = getAccessFlags(newLayout);
		VkPipelineStageFlags src_stage                 = getStageFlags(image.layout);
		VkPipelineStageFlags dst_stage                 = getStageFlags(newLayout);
		vkCmdPipelineBarrier(
		    handle,
		    src_stage, dst_stage,
		    0,
		    0, nullptr,
		    0, nullptr,
		    1, &memory_barrier);
		image.layout = newLayout;
	}

	void transitionLayout(Image &image, VkImageLayout newLayout, uint32_t baseLayer = 0, uint32_t layerCount = 1)
	{
		if (image.layout != newLayout)
		{
			imageMemoryBarrier(image, newLayout, baseLayer, layerCount);
		}
	}

	void copyToSwapchain(Image &src)
	{
		copyImage(src, gState.io.images[gState.frame->frameIndex], src.layout, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}

	void copyImage(Image &src, Image &dst)
	{
		copyImage(src, dst, src.layout, dst.layout);
	}

	void copyImage(Image &src, Image &dst, VkImageLayout srcNewLayout, VkImageLayout dstNewLayout,
	               uint32_t baseLayerSrc = 0, uint32_t layerCountSrc = 1, uint32_t baseLayerDst = 0, uint32_t layerCountDst = 1)
	{
		imageMemoryBarrier(src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, baseLayerSrc, layerCountSrc);
		imageMemoryBarrier(dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, baseLayerDst, layerCountDst);

		VkImageSubresourceLayers subresourceLayers{};
		subresourceLayers.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceLayers.mipLevel       = 0;
		subresourceLayers.baseArrayLayer = 0;
		subresourceLayers.layerCount     = 1;

		VkOffset3D offset{};
		offset.x = 0;
		offset.y = 0;
		offset.z = 0;

		VkExtent3D extent{};
		extent.width  = src.extent.width;
		extent.height = src.extent.height;
		extent.depth  = src.extent.depth;

		VkImageCopy imageCopy{};
		imageCopy.srcSubresource = subresourceLayers;
		imageCopy.srcOffset      = offset;
		imageCopy.dstSubresource = subresourceLayers;
		imageCopy.dstOffset      = offset;
		imageCopy.extent         = extent;

		vkCmdCopyImage(handle, src.img, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

		imageMemoryBarrier(src, srcNewLayout, baseLayerSrc, layerCountSrc);
		imageMemoryBarrier(dst, dstNewLayout, baseLayerDst, layerCountDst);
	}

	void move(ResourceTracker *pNewTracker)
	{
		ASSERT_TRUE(res != nullptr);
		res->move(pNewTracker);
	}

	void end()
	{
		if (stateBits & CMD_BUF_STATE_IS_RECORDING)
		{
			ASSERT_VULKAN(vkEndCommandBuffer(handle));
			stateBits &= ~CMD_BUF_STATE_IS_RECORDING;
		}
	}
	friend void commitCmdBuffers(std::vector<CmdBuffer> cmdBufs, ResourceTracker *pTracker, VkQueue queue, const SubmitSynchronizationInfo syncInfo);
	friend VkCommandBuffer vka_compatibility::getHandle(CmdBuffer cmdBuf);
  private:
};



class ComputeCmdBuffer : public CmdBuffer
{
  protected:
	  // state
	PipelineLayoutDefinition pipelineLayoutDef;
	VkPipelineBindPoint bindPoint;



  public:
	ComputeCmdBuffer();
	ComputeCmdBuffer(ResourceTracker *pTracker, VkCommandBufferUsageFlags usage, uint32_t queueIdx = 0, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	void dispatch(uint32_t x, uint32_t y, uint32_t z)
	{
		ASSERT_TRUE(stateBits & CMD_BUF_STATE_BOUND_PIPELINE);
		vkCmdDispatch(handle, x, y, z);
	}
	void dispatch(glm::uvec3 v)
	{
		ASSERT_TRUE(stateBits & CMD_BUF_STATE_BOUND_PIPELINE);
		vkCmdDispatch(handle, v.x, v.y, v.z);
	}


	void bindPipeline(ComputePipeline pipeline)
	{
		vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.getHandle());
		pipelineLayoutDef = pipeline.getState().pipelineLayoutDef;
		bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
		stateBits |= CMD_BUF_STATE_BOUND_PIPELINE;
		// State: layout, bindpoint
	}

	// global params: pipeline layout, bind point, set index
	// per write params: desc_count, desc_type
	//		buffer: buffer, offset, range
	//		image: sampler, view, layout
	//
	// write:
	// cmd buffer state: pipeline layout, bind point
	// layout + binding ->  desc_count, desc_type
	// buffer params -> vkbuffer
	// image params -> image, sampler_def = null

	template <class... Args>
	void pushDescriptors(uint32_t setIdx, Args ... args)
	{
		LOAD_CMD_VK_DEVICE(vkCmdPushDescriptorSetKHR, gState.device.logical);
		//LOAD_CMD_VK_INSTANCE(vkCmdPushDescriptorSetKHR, gState.device.instance);
		ASSERT_TRUE(stateBits & CMD_BUF_STATE_BOUND_PIPELINE);
		ASSERT_TRUE(pipelineLayoutDef.descSetLayoutDef.size() > setIdx);

		std::vector<VkWriteDescriptorSet> writes;
		std::vector<VkDescriptorBufferInfo> bufferInfos;
		std::vector<VkDescriptorImageInfo>  imageInfos;
		writes.reserve(pipelineLayoutDef.descSetLayoutDef[setIdx].bindings.size());
		bufferInfos.reserve(pipelineLayoutDef.descSetLayoutDef[setIdx].bindings.size());
		imageInfos.reserve(pipelineLayoutDef.descSetLayoutDef[setIdx].bindings.size());
		pushDescriptors(setIdx, writes, bufferInfos, imageInfos, args...);


		ASSERT_TRUE(writes.size() == pipelineLayoutDef.descSetLayoutDef[setIdx].bindings.size());
		pvkCmdPushDescriptorSetKHR(handle, bindPoint, PipelineLayout(pTracker,pipelineLayoutDef).getHandle(), setIdx, writes.size(), writes.data());
	}
	template <class... Args>
	void pushDescriptors(uint32_t                             setIdx,
	                     std::vector<VkWriteDescriptorSet>   &writes,
	                     std::vector<VkDescriptorBufferInfo> &bufferInfos,
	                     std::vector<VkDescriptorImageInfo>  &imageInfos,
	                     Buffer 							&buffer,
	                     Args... args
		)
	{
		pushDescriptors(setIdx, writes, bufferInfos, imageInfos, buffer);
		pushDescriptors(setIdx, writes, bufferInfos, imageInfos, args...);
	}
	void pushDescriptors(uint32_t                             setIdx,
	                     std::vector<VkWriteDescriptorSet>   &writes,
	                     std::vector<VkDescriptorBufferInfo> &bufferInfos,
	                     std::vector<VkDescriptorImageInfo>  &imageInfos,
	                     Buffer								 &buffer)
	{
		
		ASSERT_TRUE(pipelineLayoutDef.descSetLayoutDef[setIdx].bindings.size() > writes.size());
		bufferInfos.push_back(buffer.getDescriptorInfo());
		VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
		write.dstBinding = writes.size();
		write.descriptorCount = 1;
		write.descriptorType = pipelineLayoutDef.descSetLayoutDef[setIdx].bindings[write.dstBinding].descriptorType;
		write.pBufferInfo = &bufferInfos.back();
		writes.push_back(write);
	}

	template <class... Args>
	void pushDescriptors(uint32_t                             setIdx,
	                     std::vector<VkWriteDescriptorSet>   &writes,
	                     std::vector<VkDescriptorBufferInfo> &bufferInfos,
	                     std::vector<VkDescriptorImageInfo>  &imageInfos,
	                     Image                               &image,
	                     Args... args)
	{
		pushDescriptors(setIdx, writes, bufferInfos, imageInfos, image);
		pushDescriptors(setIdx, writes, bufferInfos, imageInfos, args...);
	}
	template <class... Args>
	void pushDescriptors(uint32_t                             setIdx,
	                     std::vector<VkWriteDescriptorSet>   &writes,
	                     std::vector<VkDescriptorBufferInfo> &bufferInfos,
	                     std::vector<VkDescriptorImageInfo>  &imageInfos,
	                     Image                               &image)
	{
		ASSERT_TRUE(pipelineLayoutDef.descSetLayoutDef[setIdx].bindings.size() > writes.size());
		VkDescriptorImageInfo imgInfo{};
		imgInfo.imageLayout = image.layout;
		imgInfo.imageView   = image.view;
		imageInfos.push_back(imgInfo);
		VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
		write.dstBinding      = writes.size();
		write.descriptorCount = 1;
		write.descriptorType  = pipelineLayoutDef.descSetLayoutDef[setIdx].bindings[write.dstBinding].descriptorType;
		write.pImageInfo      = &imageInfos.back();
		writes.push_back(write);
	}
	
	

	~ComputeCmdBuffer();

  private:
};



class UniversalCmdBuffer : public ComputeCmdBuffer
{
  public:
	UniversalCmdBuffer(); 
	UniversalCmdBuffer(ResourceTracker *pTracker, VkCommandBufferUsageFlags usage, uint32_t queueIdx = 0, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	void startRenderPass(VkRenderPass renderpass, VkFramebuffer framebuffer, std::vector<VkClearValue> clearValues, VkExtent2D extent = gState.io.extent)
	{
		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext             = nullptr;
		renderPassBeginInfo.renderPass        = renderpass;
		renderPassBeginInfo.framebuffer       = framebuffer;
		renderPassBeginInfo.renderArea.offset = {0, 0};
		renderPassBeginInfo.renderArea.extent = extent;
		renderPassBeginInfo.clearValueCount   = clearValues.size();
		renderPassBeginInfo.pClearValues      = clearValues.data();

		vkCmdBeginRenderPass(handle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport;
		viewport.x        = 0.0f;
		viewport.y        = 0.0f;
		viewport.width    = extent.width;
		viewport.height   = extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(handle, 0, 1, &viewport);
		VkRect2D scissor;
		scissor.offset = {0, 0};
		scissor.extent = {extent.width, extent.height};
		vkCmdSetScissor(handle, 0, 1, &scissor);
	}
	void endRenderPass()
	{
		vkCmdEndRenderPass(handle);
	}
	~UniversalCmdBuffer();
};



}