#include "global_state.h"
#include "mock.h"
#include "core/macros/macros.h"
namespace vka
{

//void DescriptorAllocator::init()
//{
//	ASSERT_TRUE(gState.initBits & (STATE_INIT_DEVICE_BIT | STATE_INIT_IO_BIT));
//	gState.initBits |= STATE_INIT_DESCALLOC_BIT;
//}
//void DescriptorAllocator::destroy()
//{
//	ASSERT_TRUE(gState.initBits & STATE_INIT_DESCALLOC_BIT);
//	gState.initBits &= ~STATE_INIT_DESCALLOC_BIT;
//};

//void MemAllocator::init()
//{
//	ASSERT_TRUE(gState.initBits & (STATE_INIT_DEVICE_BIT | STATE_INIT_IO_BIT));
//	gState.initBits |= STATE_INIT_MEMALLOC_BIT;
//}
//
//void MemAllocator::destroy()
//{
//	ASSERT_TRUE(gState.initBits & STATE_INIT_MEMALLOC_BIT);
//	gState.initBits &= ~STATE_INIT_MEMALLOC_BIT;
//}


//void QueryAllocator::init()
//{
//	ASSERT_TRUE(gState.initBits & (STATE_INIT_DEVICE_BIT | STATE_INIT_IO_BIT));
//	gState.initBits |= STATE_INIT_QUERYALLOC_BIT;
//}
//
//void QueryAllocator::destroy()
//{
//	ASSERT_TRUE(gState.initBits & STATE_INIT_QUERYALLOC_BIT);
//	gState.initBits &= ~STATE_INIT_QUERYALLOC_BIT;
//}

//void CmdAllocator::init()
//{
//	ASSERT_TRUE(gState.initBits & (STATE_INIT_DEVICE_BIT | STATE_INIT_IO_BIT));
//	gState.initBits |= STATE_INIT_CMDALLOC_BIT;
//}
//
//void CmdAllocator::destroy()
//{
//	ASSERT_TRUE(gState.initBits & STATE_INIT_CMDALLOC_BIT);
//	gState.initBits &= ~STATE_INIT_CMDALLOC_BIT;
//}
}        // namespace vka