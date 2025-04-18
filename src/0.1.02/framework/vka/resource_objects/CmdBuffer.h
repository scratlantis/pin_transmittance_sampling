#pragma once
#include "Resource.h"
#include "PipelineLayout.h"
#include "RenderPass.h"
namespace vka
{
class CmdBuffer_R : public Resource_T<VkCommandBuffer>
{
  public:
	CmdBuffer_R(VkCommandBuffer handle, VkCommandPool cmdPool) :
	    Resource_T<VkCommandBuffer>(handle), cmdPool(cmdPool) {}
	void free() override;
  private:
	VkCommandPool   cmdPool;
};

enum CmdBufferCapabitlityBits
{
	CMD_BUF_CAPABILITY_BIT_TRANSFER = 1 << 0,
	CMD_BUF_CAPABILITY_BIT_COMPUTE  = 1 << 1,
	CMD_BUF_CAPABILITY_BIT_GRAPHICS = 1 << 2,
	CMD_BUF_CAPABILITY_BIT_PRESENT = 1 << 3
};

enum CmdBufferCapabitlityMask
{
	CMD_BUF_CAPABILITY_MASK_NONE = 0,
	CMD_BUF_CAPABILITY_MASK_TRANSFER =
	    CMD_BUF_CAPABILITY_BIT_TRANSFER,
	CMD_BUF_CAPABILITY_MASK_COMPUTE =
	    CMD_BUF_CAPABILITY_BIT_TRANSFER |
	    CMD_BUF_CAPABILITY_BIT_COMPUTE,
	CMD_BUF_CAPABILITY_MASK_UNIVERSAL =
	    CMD_BUF_CAPABILITY_BIT_TRANSFER |
	    CMD_BUF_CAPABILITY_BIT_COMPUTE |
	    CMD_BUF_CAPABILITY_BIT_GRAPHICS |
	    CMD_BUF_CAPABILITY_BIT_PRESENT
};

enum CmdBufferStateBits
{
	CMD_BUF_STATE_BITS_RECORDING   = 1 << 0,
	CMD_BUF_STATE_BITS_BOUND_PIPELINE = 1 << 1
};

enum RenderStateActionBits
{
	RENDER_STATE_ACTION_BIT_START_RENDER_PASS  = 1 << 0,
	RENDER_STATE_ACTION_BIT_END_RENDER_PASS    = 1 << 1,
	RENDER_STATE_ACTION_BIT_NEXT_SUBPASS       = 1 << 2,
	RENDER_STATE_ACTION_BIT_BIND_PIPELINE      = 1 << 3,
	RENDER_STATE_ACTION_BIT_BIND_VERTEX_BUFFER = 1 << 4,
	RENDER_STATE_ACTION_BIT_BIND_INDEX_BUFFER  = 1 << 5,
};

class Buffer_I;
struct RenderState
{
	// -> RenderPassBegin
	VkFramebuffer           framebuffer;
	std::vector<ClearValue> clearValues;
	VkRect2D_OP             renderArea;
	VkRenderPass            renderPass;

	// -> Subpass
	uint32_t subpassIdx;

	// -> BindPipeline
	VkPipeline               pipeline;
	PipelineLayoutDefinition pipelineLayoutDef;
	VkPipelineBindPoint      bindPoint;

	// -> Bind Buffers
	std::vector<Buffer_I *> vertexBuffers;
	Buffer_I               *indexBuffer;

	uint32_t calculateDifferenceBits(const RenderState &other) const
	{
		uint32_t diffBits = 0;
		// todo subpasses
		if (renderPass == VK_NULL_HANDLE)
		{
			diffBits |= RENDER_STATE_ACTION_BIT_START_RENDER_PASS;
		}
		else if (other.renderPass != renderPass || !cmpVector(clearValues, other.clearValues) || renderArea != other.renderArea)
		{
			diffBits |= RENDER_STATE_ACTION_BIT_END_RENDER_PASS;
			diffBits |= RENDER_STATE_ACTION_BIT_START_RENDER_PASS;
		}
		if (pipeline != other.pipeline)
		{
			diffBits |= RENDER_STATE_ACTION_BIT_BIND_PIPELINE;
		}
		if (!cmpVector(vertexBuffers, other.vertexBuffers))
		{
			diffBits |= RENDER_STATE_ACTION_BIT_BIND_VERTEX_BUFFER;
		}
		if (indexBuffer != other.indexBuffer)
		{
			diffBits |= RENDER_STATE_ACTION_BIT_BIND_INDEX_BUFFER;
		}
		return diffBits;
	}

	std::vector<VkClearValue> getClearValues() const;
};

class CmdBuffer_I : public Resource_T<VkCommandBuffer>
{
  protected:
	CmdBuffer_R             *res = nullptr;
	bool        hasMemoryOwnership = false;

  public:
	// State
	uint32_t                 stateBits;
	CmdBufferCapabitlityMask capability;

	

	RenderState renderState;

	CmdBuffer_I(IResourcePool* pPool) :
		Resource_T<VkCommandBuffer>(VK_NULL_HANDLE)
	{
		renderState       = {};
		capability = CMD_BUF_CAPABILITY_MASK_NONE;
		stateBits         = 0;
		this->pPool       = pPool;
		track(pPool);
	}
	CmdBuffer_I(IResourcePool *pPool, CmdBufferCapabitlityMask capability, VkCommandBufferUsageFlags usage, VkCommandBufferLevel level, uint32_t poolIdx) :
	    CmdBuffer_I(pPool)
	{
		createHandles(capability, usage, level, poolIdx);
	}
	~CmdBuffer_I()
	{
		free();
	}

	void CmdBuffer_I::end()
	{
		if (stateBits & CMD_BUF_STATE_BITS_RECORDING)
		{
			VK_CHECK(vkEndCommandBuffer(handle));
			stateBits &= ~CMD_BUF_STATE_BITS_RECORDING;
		}
	}

	void   createHandles(CmdBufferCapabitlityMask capability, VkCommandBufferUsageFlags usage, VkCommandBufferLevel level, uint32_t poolIdx);
	hash_t hash() const override;
	void free() override;
	void track(IResourcePool *pPool) override;
};
}        // namespace vka