#include "Resource.h"
#include <vka/state_objects/global_state.h>

namespace vka
{
// _I internal
// _R resource

class Buffer_R : public Resource
{
  public:
	    Buffer_R(){};
	virtual void *map(uint32_t offset, uint32_t size) = 0;
	virtual void  unmap()                             = 0;
};


class BufferLocal_R : public Buffer_R
{
  public:
	BufferLocal_R(void *handle, uint64_t size) :
	    handle(handle), size(size)
	{
	}
	// Resource interface
	hash_t hash() const
	{
		return (hash_t) this->handle;
	}
	void free()
	{
		std::free(handle);
	}
	// Buffer_R interface
	void* map(uint32_t offset, uint32_t size)
	{
		VKA_ASSERT(offset + size < this->size);
		return (char *) handle + offset;
	}
	void unmap(){}
  protected:
	bool _equals(Resource const &other) const
	{
		if (typeid(*this) != typeid(other))
			return false;
		else
		{
			auto &other_ = static_cast<BufferLocal_R const &>(other);
			return this->handle == other_.handle;
		}
	};
  private:
	void *const  handle;
	const uint64_t size;
};


class BufferVK_R : public Buffer_R
{
  public:
	BufferVK_R(VkBuffer handle, VkDeviceMemory deviceMemory)
	{
		this->handle       = handle;
		this->deviceMemory = deviceMemory;
	}
	hash_t hash() const
	{
		return (hash_t) this->handle;
	}
	bool  _equals(Resource const &other) const
	{
		if (typeid(*this) != typeid(other))
			return false;
		else
		{
			auto &other_ = static_cast<BufferVK_R const &>(other);
			return this->handle == other_.handle;
		}
	};
	void *map(uint32_t offset, uint32_t size)
	{
		void *data;
		gState.memAlloc.mapMemory(deviceMemory, offset, size, &data);
		return data;
	}
	void unmap()
	{
		gState.memAlloc.unmapMemory(deviceMemory);
	}

  protected:
	void free()
	{
		gState.memAlloc.destroyBuffer(handle, deviceMemory);
	}

  private:
	VkBuffer       handle;
	VkDeviceMemory deviceMemory;
};

class BufferVMA_R : public Buffer_R
{
  public:
	BufferVMA_R(VkBuffer handle, VmaAllocation allocation)
	{
		this->handle     = handle;
		this->allocation = allocation;
	}
	hash_t hash() const
	{
		return (hash_t) this->handle;
	}
	bool _equals(Resource const &other) const
	{
		if (typeid(*this) != typeid(other))
			return false;
		else
		{
			auto &other_ = static_cast<BufferVMA_R const &>(other);
			return this->handle == other_.handle;
		}
	};
	void *map(uint32_t offset, uint32_t size)
	{
		void *data;
		gState.memAlloc.mapMemory(allocation, &data);
		data = (char *) data + offset;
		return data;
	}
	void unmap()
	{
		gState.memAlloc.unmapMemory(allocation);
	}

  protected:
	void free()
	{
		gState.memAlloc.destroyBuffer(handle, allocation);
	}

  private:
	VkBuffer      handle;
	VmaAllocation allocation;
};

class BufferView_R : public Resource
{
  public:
	BufferView_R(VkBufferView handle)
	{
		this->handle = handle;
	}
	hash_t hash() const
	{
		return (hash_t) this->handle;
	}
	bool _equals(Resource const &other) const
	{
		if (typeid(*this) != typeid(other))
			return false;
		else
		{
			auto &other_ = static_cast<BufferView_R const &>(other);
			return this->handle == other_.handle;
		}
	};

  protected:
	void free()
	{
		vkDestroyBufferView(gState.device.logical, handle, nullptr);
	}

  private:
	VkBufferView handle;
};

enum class BufferType
{
	NONE,
	LOCAL,
	VK,
	VMA
};

union MemoryProperty
{
	VmaMemoryUsage        vma;
	VkMemoryPropertyFlags vk;
};

union BufferHandle
{
	VkBuffer    vk;
	const void *local;
};



class Buffer_I : Resource
{
  protected:
	Buffer_R          *res      = nullptr;
	BufferView_R      *viewRes  = nullptr;
	bool               isMapped        = false;
	bool               hasMemoryOwnership = false;
  public:
	BufferType         type;
	VkDeviceSize       size;
	VkBufferUsageFlags usage;
	MemoryProperty     memProperty;
	BufferHandle       handle;
	VkBufferView       viewHandle;

  private:
	Buffer_I()
	{
		type               = BufferType::NONE;
		usage              = 0;
		memProperty.vk     = 0;
		handle.vk          = VK_NULL_HANDLE;
		viewHandle         = VK_NULL_HANDLE;
		pPool              = nullptr;
		size               = 0;
		hasMemoryOwnership = false;
	};
  public:
	~Buffer_I()
	{
		destroy();
	}
	Buffer_I(BufferType type, VkDeviceSize size, VkBufferUsageFlags usage, MemoryProperty memProperty) :
	    Buffer_I()
	{
		create(type, size, usage, memProperty);
	}
	void create(BufferType type, VkDeviceSize size, VkBufferUsageFlags usage, MemoryProperty memProperty)
	{
		if (!hasMemoryOwnership)
		{
			this->type        = type;
			this->usage       = usage;
			this->memProperty = memProperty;
			this->size        = size;
			switch (type)
			{
				case vka::BufferType::LOCAL:
					void *heapPointer = std::malloc(size);
					handle.local      = heapPointer;
					res               = new BufferLocal_R(heapPointer, size);
					break;
				case vka::BufferType::VK:
					VkDeviceMemory deviceMemory;
					gState.memAlloc.createBufferDedicated(size, usage, memProperty.vk, &handle.vk, &deviceMemory);
					res = new BufferVK_R(handle.vk, deviceMemory);
					break;
				case vka::BufferType::VMA:
					VmaAllocation allocation;
					gState.memAlloc.createBuffer(size, usage, memProperty.vma, &handle.vk, &allocation);
					res = new BufferVMA_R(handle.vk, allocation);
					break;
				default:
					vka::printVka("Invalid buffer type\n");
					DEBUG_BREAK;
					break;
			}
			hasMemoryOwnership = true;
		}
		else
		{
			vka::printVka("Cant recreate buffer with memory ownership\n");
			DEBUG_BREAK;
		}
	}
	void destroy()
	{
		if (!hasMemoryOwnership)
		{
			type           = BufferType::NONE;
			usage          = 0;
			memProperty.vk = 0;
			handle.vk      = VK_NULL_HANDLE;
			viewHandle     = VK_NULL_HANDLE;
			pPool          = nullptr;
			size           = 0;
		}
		else
		{
			vka::printVka("Cant free buffer with memory ownership\n");
			DEBUG_BREAK;
		}
	}
	void recreate(BufferType type, VkDeviceSize size, VkBufferUsageFlags usage, MemoryProperty memProperty, bool maintainData = false)
	{
		if (maintainData)
		{
			Buffer_R* oldRes = res;
			VkDeviceSize minDataSize = std::min(this->size, size);
			destroy();
			create(type, size, usage, memProperty);
			void *oldData = oldRes->map(0, minDataSize);
			void *newData = map(0, minDataSize);
			std::memcpy(newData, oldData, minDataSize);
			unmap();
			oldRes->unmap();

		}
		destroy();
		create(type, size, usage, memProperty);
	}
	void track(ResourcePool *pPool) override
	{
		if (!pPool)
		{
			vka::printVka("Null resource pool\n");
			DEBUG_BREAK;
			return;
		}
		if (viewRes)
		{
			viewRes->track(pPool);
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
		if (viewRes)
		{
			viewRes->garbageCollect();
		}
		if (res)
		{
			res->garbageCollect();
		}
		if (type != BufferType::NONE)
		{
			track(&gState.frame->stack);
		}
	}
	// For resource interface
	void free(){}
	hash_t hash() const
	{
		return res->hash();
	}
	bool _equals(Resource const &other) const
	{
		if (typeid(*this) != typeid(other))
			return false;
		else
		{
			auto &other_ = static_cast<Buffer_I const &>(other);
			VKA_ASSERT(this->type != BufferType::NONE && other_.type != BufferType::NONE);
			return *res == *other_.res;
		}
	};

	void* map(uint32_t offset, uint32_t size)
	{
		isMapped = true;
		return res->map(offset, size);
	}
	void unmap()
	{
		isMapped = false;
		res->unmap();
	}
};

typedef Buffer_I* VkaBuffer;

struct VkaBufferCreateInfo
{
	BufferType         type;
	VkDeviceSize       size;
	VkBufferUsageFlags usage;
	MemoryProperty     memProperty;
};

void vkaReloadBuffer(VkaBuffer buffer, VkaBufferCreateInfo& ci, bool maintainData)
{
	if (buffer->type != ci.type || buffer->size != ci.size || buffer->usage != ci.usage ||
		std::memcmp(&buffer->memProperty,&ci.memProperty, sizeof(MemoryProperty)) != 0) 
	{
		buffer->garbageCollect();
		buffer->recreate(ci.type, ci.size, ci.usage, ci.memProperty, maintainData);
	}
}

void vkaGetBufferCI(VkaBuffer buffer, VkaBufferCreateInfo &ci)
{
	ci.type        = buffer->type;
	ci.size        = buffer->size;
	ci.usage       = buffer->usage;
	ci.memProperty = buffer->memProperty;
}

bool vkaAddBufferUsage(VkaBuffer buffer, VkBufferUsageFlags usage)
{
	if ((buffer->usage & usage) != usage)
	{
		VkaBufferCreateInfo ci;
		vkaGetBufferCI(buffer, ci);
		ci.usage |= usage;
		vkaReloadBuffer(buffer, ci, true);
		return true;
	}
	return false;
}
//void vkaCopyBuffer(VkaCommandBuffer cmdBuf, VkaBuffer src, VkaBuffer dst, VkBufferCopy copy)
//{
//	vkaAddBufferUsage(src, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
//	vkaAddBufferUsage(dst, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
//	// Todo: Implement command buffer
//}


}