#pragma once
#include "Resource.h"
#include "PipelineLayout.h"
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

class CmdBuffer_I : public Resource_T<VkCommandBuffer>
{
  protected:
	CmdBuffer_R             *res;
	bool        hasMemoryOwnership = false;

  public:
	// State
	uint32_t                 stateBits;
	CmdBufferCapabitlityMask capability;
	PipelineLayoutDefinition pipelineLayoutDef;
	VkPipelineBindPoint      bindPoint;
	CmdBuffer_I() :
		Resource_T<VkCommandBuffer>(VK_NULL_HANDLE)
	{
		pipelineLayoutDef = {};
		bindPoint         = VK_PIPELINE_BIND_POINT_MAX_ENUM;
		capability = CMD_BUF_CAPABILITY_MASK_NONE;
		stateBits         = 0;
	}
	CmdBuffer_I(CmdBufferCapabitlityMask capability, VkCommandBufferUsageFlags usage, VkCommandBufferLevel level, uint32_t poolIdx) :
	    CmdBuffer_I()
	{
		create(capability, usage, level, poolIdx);
	}
	~CmdBuffer_I()
	{
		free();
	}

	void   create(CmdBufferCapabitlityMask capability, VkCommandBufferUsageFlags usage, VkCommandBufferLevel level, uint32_t poolIdx);
	hash_t hash() const override;
	void free() override;
	void track(IResourcePool *pPool) override;
};
}        // namespace vka