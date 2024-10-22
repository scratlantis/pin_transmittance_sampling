#include "TimeQueryManager.h"
#include <vka/globals.h>
namespace vka
{
TimeQueryManager::TimeQueryManager(IResourcePool *pPool, uint32_t queryCount)
{
	timings.resize(queryCount);
	for (size_t i = 0; i < queryCount; i++)
	{
		timings[i] = 0.0f;
	}
	queryResults.resize(queryCount * 2);
	VkQueryPoolCreateInfo queryPoolCI{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
	queryPoolCI.queryType  = VK_QUERY_TYPE_TIMESTAMP;
	queryPoolCI.queryCount = queryResults.size();
	queryPool              = createQueryPool(pPool, queryPoolCI);
}

void TimeQueryManager::cmdResetQueryPool(CmdBuffer cmdBuffer)
{
	cmdClearState(cmdBuffer);
	vkCmdResetQueryPool(cmdBuffer->getHandle(), queryPool, 0, queryResults.size());
}

TimeQueryManager::~TimeQueryManager()
{
}

void TimeQueryManager::startTiming(CmdBuffer cmdBuffer, uint32_t queryID, VkPipelineStageFlagBits stage)
{
	vkCmdWriteTimestamp(cmdBuffer->getHandle(), stage, queryPool, queryID * 2);
}

void TimeQueryManager::endTiming(CmdBuffer cmdBuffer, uint32_t queryID, VkPipelineStageFlagBits stage)
{
	vkCmdWriteTimestamp(cmdBuffer->getHandle(), stage, queryPool, queryID * 2 + 1);
}

bool TimeQueryManager::updateTimings()
{
	if (firstUse)
	{
		firstUse = false;
		return false;
	}
	VkResult result = vkGetQueryPoolResults(
	    gState.device.logical,
	    queryPool,
	    0,
	    queryResults.size(),
	    queryResults.size() * sizeof(uint64_t),
	    queryResults.data(),
	    static_cast<VkDeviceSize>(sizeof(uint64_t)),
	    VK_QUERY_RESULT_64_BIT);
	VkPhysicalDeviceProperties physicalDeviceProps{};
	vkGetPhysicalDeviceProperties(gState.device.physical, &physicalDeviceProps);
	if (result != VK_NOT_READY)
	{
		for (size_t i = 0; i < timings.size(); i++)
		{
			uint64_t t_ns      = (queryResults[2 * i + 1] - queryResults[2 * i]) * physicalDeviceProps.limits.timestampPeriod;
			float    t_float   = static_cast<float>(t_ns) / 1000000.f;
			timings[i] = t_float;
		}
	}
	return result != VK_NOT_READY;
}
}        // namespace vka
