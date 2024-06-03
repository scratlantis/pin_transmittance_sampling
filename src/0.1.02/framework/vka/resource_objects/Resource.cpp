#include "Resource.h"
#include <vka/state_objects/global_state.h>
namespace vka
{

void Resource::garbageCollect()
{
	track(&gState.frame->stack);
}
} // namespace vka