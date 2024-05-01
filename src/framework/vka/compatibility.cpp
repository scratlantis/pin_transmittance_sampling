#include "compatibility.h"
#include "combined_resources/CmdBuffer.h"
using namespace vka;
namespace vka_compatibility
{
VkCommandBuffer getHandle(CmdBuffer cmdBuf)
{
	return cmdBuf.handle;
}
}        // namespace vka_compatibility
