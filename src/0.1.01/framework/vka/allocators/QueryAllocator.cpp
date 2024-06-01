#include "QueryAllocator.h"
#include "../global_state.h"
namespace vka
{
void QueryAllocator::init()
{
	ASSERT_TRUE(gState.initBits & (STATE_INIT_DEVICE_BIT | STATE_INIT_IO_BIT));
	gState.initBits |= STATE_INIT_QUERYALLOC_BIT;
}

void QueryAllocator::destroy()
{
	ASSERT_TRUE(gState.initBits & STATE_INIT_QUERYALLOC_BIT);
	gState.initBits &= ~STATE_INIT_QUERYALLOC_BIT;
}
}