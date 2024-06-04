#include "Resource.h"
#include <vka/state_objects/global_state.h>
namespace vka
{

void Resource::garbageCollect()
{
	track(&gState.frame->stack);
}

void Resource::track(ResourcePool *pPool)
{
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
}

void CachableResource::track(ResourcePool *pPool)
{
	if (pCache == nullptr)
	{
		Resource::track(pPool);
	}
	else
	{
		printVka("CachableResource::track() called on a resource that is already cached\n");
		DEBUG_BREAK;
	}
}

}        // namespace vka