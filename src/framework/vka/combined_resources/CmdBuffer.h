#pragma once
#include "../resources/Resource.h"
#include "../resources/ComputePipeline.h"
namespace vka
{

enum CmdBufferStateBits
{
	CMD_BUF_STATE_BOUND_PIPELINE = 0x1
};

class CmdBuffer;
void commitCmdBuffers(std::vector<CmdBuffer> cmdBufs, ResourceTracker *pTracker, VkQueue queue, const SubmitSynchronizationInfo syncInfo = {});

class CmdBuffer
{
  protected:
	NonUniqueResource *res;
	VkCommandBuffer handle;
	bool            isRecording;
	uint32_t        stateBits;

  public:
	CmdBuffer();
	~CmdBuffer();

	template <class T>
	void uploadData(Buffer dst, T dataStruct, ResourceTracker *garbageTracker = &gState.frame->stack)
	{
		VkDeviceSize size = sizeof(T);
		uploadData(dst, 0, &dataStruct, size, garbageTracker)
	}

	template <class T>
	Buffer uploadData(T dataStruct, ResourceTracker *bufferTracker, VkBufferUsageFlagBits usageFlags, ResourceTracker *garbageTracker = &gState.frame->stack)
	{
		VkDeviceSize size = sizeof(T);
		Buffer       dst  = BufferVma(bufferTracker, size, usageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);	
		uploadData(dst, dataStruct);
	}

	void uploadData(Buffer dst, uint32_t dstOffset, const void *data, VkDeviceSize size, ResourceTracker *garbageTracker = &gState.frame->stack)
	{
		Buffer stagingBuf = BufferVma(garbageTracker, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		stagingBuf.write(data, size);
		VkBufferCopy copyRegion{0, dstOffset, size};
		vkCmdCopyBuffer(handle, stagingBuf.buf, dst.buf, 1, &copyRegion);
	}

	void move(ResourceTracker *pNewTracker)
	{
		ASSERT_TRUE(res != nullptr);
		res->move(pNewTracker);
	}

	void end()
	{
		if (isRecording)
		{
			ASSERT_VULKAN(vkEndCommandBuffer(handle));
			isRecording = false;
		}
	}
	friend void commitCmdBuffers(std::vector<CmdBuffer> cmdBufs, ResourceTracker *pTracker, VkQueue queue, const SubmitSynchronizationInfo syncInfo);





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
		pipelineLayoutDef = pipeline.pipelineState.pipelineLayoutDef;
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
		ASSERT_TRUE(stateBits & CMD_BUF_STATE_BOUND_PIPELINE);
		ASSERT_TRUE(pipelineLayoutDef.descSetLayoutDef.size() > setIdx);

		std::vector<VkWriteDescriptorSet> writes;
		std::vector<VkDescriptorBufferInfo> bufferInfos;
		std::vector<VkDescriptorImageInfo>  imageInfos;
		writes.reserve(pipelineLayoutDef.descSetLayoutDef[setIdx].bindings.size());
		bufferInfos.reserve(pipelineLayoutDef.descSetLayoutDef[setIdx].bindings.size());
		imageInfos.reserve(pipelineLayoutDef.descSetLayoutDef[setIdx].bindings.size());
		pushDescriptors(writes, bufferInfos, imageInfos, args...);


		ASSERT_TRUE(writes.size() == pipelineLayoutDef.descSetLayoutDef[setIdx].bindings.size());
		vkCmdPushDescriptorSetKHR(handle, bindPoint, pipelineLayoutDef.getHandle(), setIdx, writes.size(), writes.data());
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
	                     const Buffer                        &buffer)
	{
		
		ASSERT_TRUE(pipelineLayoutDef.descSetLayoutDef[setIdx].bindings.size() > writes.size());
		bufferInfos.push_back(buffer.getDescriptorInfo());
		VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
		write.dstBinding = writes.size();
		write.descriptorCount = 1;
		write.descriptorType = pipelineLayoutDef.descSetLayoutDef[setIdx].bindings[write.dstBinding].descriptorType;
		write.pBufferInfo = bufferInfos.end()._Ptr;
		writes.push_back(write);
	}
	template <class T, class... Args>
	void pushDescriptors(uint32_t                             setIdx,
	                     std::vector<VkWriteDescriptorSet>   &writes,
	                     std::vector<VkDescriptorBufferInfo> &bufferInfos,
	                     std::vector<VkDescriptorImageInfo>  &imageInfos,
	                     T									 &dataStruct,
	                     Args... args)
	{
		pushDescriptors(setIdx, writes, bufferInfos, imageInfos, dataStruct);
		pushDescriptors(setIdx, writes, bufferInfos, imageInfos, args...);
	}
	template <class T, class... Args>
	void pushDescriptors(uint32_t                             setIdx,
	                     std::vector<VkWriteDescriptorSet>   &writes,
	                     std::vector<VkDescriptorBufferInfo> &bufferInfos,
	                     std::vector<VkDescriptorImageInfo>  &imageInfos,
	                     T                                   &dataStruct)
	{
		ASSERT_TRUE(pipelineLayoutDef.descSetLayoutDef[setIdx].bindings.size() > writes.size());
		VkBufferUsageFlags usage = 0;
		switch (pipelineLayoutDef.descSetLayoutDef[setIdx].bindings[write.dstBinding].descriptorType)
		{
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
				break;
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
				break;
			default:
				__debugbreak();
				break;
		}
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uploadData(dataStruct, gState.frame->stack, usage).buf;
		bufferInfos.push_back(bufferInfo);
		VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
		write.dstBinding      = writes.size();
		write.descriptorCount = 1;
		write.descriptorType  = pipelineLayoutDef.descSetLayoutDef[setIdx].bindings[write.dstBinding].descriptorType;
		write.pBufferInfo     = bufferInfos.end()._Ptr;
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
		ASSERT_TRUE(pipelineLayoutDef.descSetLayoutDef[setIdx].bindings.size() > writes.size());
		VkDescriptorImageInfo imgInfo{};
		imgInfo.imageLayout = image.layout;
		imgInfo.imageView = image.view;
		imageInfos.push_back(imgInfo);
		VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
		write.dstBinding      = writes.size();
		write.descriptorCount = 1;
		write.descriptorType  = pipelineLayoutDef.descSetLayoutDef[setIdx].bindings[write.dstBinding].descriptorType;
		write.pImageInfo     = imageInfos.end()._Ptr;
		writes.push_back(write);
	}
	template <class... Args>
	void pushDescriptors(uint32_t                             setIdx,
	                     std::vector<VkWriteDescriptorSet>   &writes,
	                     std::vector<VkDescriptorBufferInfo> &bufferInfos,
	                     std::vector<VkDescriptorImageInfo>  &imageInfos,
	                     Image                               &image)
	{
		pushDescriptors(setIdx, writes, bufferInfos, imageInfos, image);
		pushDescriptors(setIdx, writes, bufferInfos, imageInfos, args...);
	}

	~ComputeCmdBuffer();

  private:
};



class UniversalCmdBuffer : public ComputeCmdBuffer
{
  public:
	UniversalCmdBuffer(); 
	UniversalCmdBuffer(ResourceTracker *pTracker, VkCommandBufferUsageFlags usage, uint32_t queueIdx = 0, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	~UniversalCmdBuffer();
};



}