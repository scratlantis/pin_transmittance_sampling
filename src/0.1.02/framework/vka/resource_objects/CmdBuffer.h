#pragma once
#include "Resource.h"
#include <vka/state_objects/global_state.h>
namespace vka
{
class CmdBuffer_R : public Resource
{
  public:
	CmdBuffer_R(VkCommandBuffer handle, VkCommandPool cmdPool) :
	    handle(handle), cmdPool(cmdPool)
	{
	}
	hash_t hash() const override
	{
		return (hash_t) this->handle;
	}

	void free() override
	{
		vkFreeCommandBuffers(gState.device.logical, cmdPool, 1, &handle);
	}

  protected:
	bool _equals(Resource const &other) const override
	{
		if (typeid(*this) != typeid(other))
			return false;
		else
		{
			auto &other_ = static_cast<CmdBuffer_R const &>(other);
			return this->handle == other_.handle;
		}
	};


  private:
	VkCommandBuffer handle;
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

class CmdBuffer_I : public Resource
{
	CmdBuffer_R             *res;
	VkCommandBuffer          handle;
	CmdBufferCapabitlityMask capability;

	// State
	uint32_t stateBits = 0;
	//PipelineLayoutDefinition pipelineLayoutDef;
	VkPipelineBindPoint      bindPoint;
	bool        hasMemoryOwnership = false;

  public:
	CmdBuffer_I(CmdBufferCapabitlityMask capability, VkCommandBufferUsageFlags usage, VkCommandBufferLevel level, uint32_t poolIdx)
	{
		VkCommandPool pool;
		switch (capability)
		{
			case CMD_BUF_CAPABILITY_MASK_TRANSFER:
				if (gState.cmdAlloc.createCmdBuffersCompute(poolIdx, level, 1, handle, pool))
				{
					break;
				}
			case CMD_BUF_CAPABILITY_MASK_COMPUTE:
				if (gState.cmdAlloc.createCmdBuffersCompute(poolIdx, level, 1, handle, pool))
				{
					break;
				}
			case CMD_BUF_CAPABILITY_MASK_UNIVERSAL:
				if (gState.cmdAlloc.createCmdBuffersUniversal(poolIdx, level, 1, handle, pool))
				{
					break;
				}
			default:
				printVka("Unable to allocate command buffer with capability %d, from pool index %d\n", capability, poolIdx);
				DEBUG_BREAK;
				break;
		}
		VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
		beginInfo.flags = usage;
		CHECK_TRUE(vkBeginCommandBuffer(handle, &beginInfo));
		stateBits |= CMD_BUF_STATE_BITS_RECORDING;
	}
	hash_t hash() const override
	{
		return (hash_t) this->handle;
	}

	void destroy()
	{
		if (!hasMemoryOwnership)
		{
			res = nullptr;
			handle = VK_NULL_HANDLE;
			capability = CMD_BUF_CAPABILITY_MASK_NONE;
			uint32_t stateBits = 0;
			// PipelineLayoutDefinition pipelineLayoutDef;
		}
		else
		{
			vka::printVka("Cant free CmdBuffer with memory ownership\n");
			DEBUG_BREAK;
		}
	}

	void track(ResourcePool *pPool) override
	{
		if (!pPool)
		{
			vka::printVka("Null resource pool\n");
			DEBUG_BREAK;
			return;
		}
		if (res)
		{
			res->track(pPool);
		}
		if (this->pPool)
		{
			if (this->pPool == pPool)
			{
				return;
			}

			if (this->pPool->remove(this))
			{
				this->pPool = pPool;
				this->pPool->add(this);
			}
			else
			{
				printVka("Resource not found in assigned pool\n");
				DEBUG_BREAK;
			}
		}
		else
		{
			this->pPool = pPool;
			this->pPool->add(this);
		}
		hasMemoryOwnership = false;
	}
	void garbageCollect() override
	{
		if (res)
		{
			res->garbageCollect();
			track(&gState.frame->stack);
		}
	}


  protected:
	bool _equals(Resource const &other) const override
	{
		if (typeid(*this) != typeid(other))
			return false;
		else
		{
			auto &other_ = static_cast<CmdBuffer_I const &>(other);
			return this->handle == other_.handle;
		}
	};
};
typedef CmdBuffer_I *VkaCommandBuffer;




}        // namespace vka