#include "DescriptorAllocator.h"
#include "../global_state.h"
namespace vka
{

void DescriptorAllocator::init()
{
	ASSERT_TRUE(gState.initBits & (STATE_INIT_DEVICE_BIT | STATE_INIT_IO_BIT));
	gState.initBits |= STATE_INIT_DESCALLOC_BIT;
}
void DescriptorAllocator::destroy()
{
	ASSERT_TRUE(gState.initBits & STATE_INIT_DESCALLOC_BIT);
	gState.initBits &= ~STATE_INIT_DESCALLOC_BIT;
};

}