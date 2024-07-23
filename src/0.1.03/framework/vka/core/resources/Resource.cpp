#include "Resource.h"
#include <vka/globals.h>
namespace vka
{
void Resource::garbageCollect()
{
	track(gState.frame->stack);
}

void Resource::track(IResourcePool *pPool)
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

}        // namespace vka